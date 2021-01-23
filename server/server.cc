#include "server.h"

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

#include "../common/news_item.h"

namespace server {
namespace {
constexpr int kListenPendingQueue = 3;
constexpr int kReadTimeOutSec = 1;
constexpr int kBufferSize = 1024;

void InsertClient(std::vector<ClientProxy>& clients,
                  const ClientProxy& client) {
  for (ClientProxy& c : clients) {
    if (c.IsConnected()) continue;
    c.socket = client.socket;
    c.address = client.address;
    return;
  }
  clients.push_back(client);
}

// Adds the client in the wait queue to list of clients that are
// actively being listened to.
void AddWaitingClients(std::queue<ClientProxy>& client_wait_queue,
                       std::vector<ClientProxy>& clients) {
  while (client_wait_queue.size()) {
    const ClientProxy client = client_wait_queue.front();
    client_wait_queue.pop();
    if (!client.IsConnected()) continue;
    InsertClient(clients, client);
    std::cout << "Reader thread is listening to new client: " << client.socket
              << std::endl;
  }
}
}  // namespace

Server::Server(const std::string& config_file_name)
    : config_file_reader_(config_file_name) {
  InitializeSubscriberConfig();
}

Server::~Server() {
  listening_ = false;
  reader_thread_->join();
}

bool Server::OpenSocket(int port) {
  int opt = 1;
  socket_ = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_ == 0) {
    std::cerr << "Failed to create socket" << std::endl;
    return false;
  }

  if (setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                 sizeof(opt))) {
    std::cerr << "Failed to setsockopt" << std::endl;
    return false;
  }

  address_.sin_family = AF_INET;
  address_.sin_addr.s_addr = INADDR_ANY;
  address_.sin_port = htons(port);

  // Forcefully attaching socket to the port 8080
  if (bind(socket_, (struct sockaddr*)&address_, sizeof(address_)) < 0) {
    std::cerr << "Failed to bind" << std::endl;
    return false;
  }
  if (listen(socket_, kListenPendingQueue) < 0) {
    std::cerr << "Failed to listen" << std::endl;
    return false;
  }

  return true;
}

void Server::StartListening() {
  InitializeReaderThread();

  fd_set fdset;
  FD_ZERO(&fdset);
  FD_SET(socket_, &fdset);
  while (true) {
    // Listen for client connections.
    int activity = select(socket_ + 1, &fdset, nullptr, nullptr, nullptr);

    if (activity < 0) {
      std::cerr << "Select Error" << std::endl;
      break;
    }

    // If its not the server socket, then ignore.
    if (!FD_ISSET(socket_, &fdset)) {
      std::cerr << "Message received on non server socket" << std::endl;
      continue;
    }
    ClientProxy client;
    int addrlen = sizeof(client.address);

    client.socket = accept(socket_, (struct sockaddr*)&client.address,
                           (socklen_t*)&addrlen);

    if (client.socket < 0) {
      std::cerr << "Failed to accept client" << std::endl;
      continue;
    }

    std::cout << "New connection: " << client.socket
              << " with IP: " << inet_ntoa(client.address.sin_addr)
              << " and port: " << ntohs(client.address.sin_port) << std::endl;

    HandleNewClient(client);
  }
  listening_ = false;
  TerminateReaderThread();
}

void Server::InitializeSubscriberConfig() {
  common::SubscriberItem item;

  while (config_file_reader_.GetNextLineAsSubscriberItem(item)) {
    auto it = subscribers_.find(item.subscriber_id);
    if (it == subscribers_.end()) {
      subscribers_.insert({item.subscriber_id,
          std::make_unique<Subscriber>(item.subscriber_id)});
    }
    auto& subscriber = subscribers_.at(item.subscriber_id);
    subscriber->AddInterest(item.category);
  }
}

void Server::TerminateReaderThread() {
  reader_thread_->join();
  reader_thread_.reset(nullptr);
}

void Server::HandleNewClient(ClientProxy client) {
  {
    std::unique_lock<std::mutex> lck(clients_mtx_);
    clients_wait_queue_.push(client);
  }
  send(client.socket, "ACK", 3, 0);
}

void Server::InitializeReaderThread() {
  // Initialize a thread that will read responses from clients.
  listening_ = true;
  reader_thread_ =
      std::make_unique<std::thread>(&Server::ReadFromClientLoop, this);
}

void Server::ReadFromClientLoop() {
  std::cout << "Thread is beginning to read in loop" << std::endl;
  fd_set fd_read_set;
  FD_ZERO(&fd_read_set);

  struct timeval timeout;
  timeout.tv_sec = kReadTimeOutSec;
  timeout.tv_usec = 0;

  char buffer[kBufferSize] = {0};

  std::queue<ClientProxy*> send_acks_to;

  while (listening_) {
    // Send "ACK" to clients so they send the next message.
    SendAcksToClient(send_acks_to);

    {
      // Check if any new client has connected since last iteration and start
      // listening to them as well.
      std::unique_lock<std::mutex> lck(clients_mtx_);
      AddWaitingClients(clients_wait_queue_, clients_);
    }

    // Add clients to the |fd_read_set| so select() can listen to fd changes.
    int max_socket = 0;
    FD_ZERO(&fd_read_set);
    for (const auto& client : clients_) {
      if (!client.IsConnected()) continue;
      FD_SET(client.socket, &fd_read_set);
      max_socket = std::max(max_socket, client.socket);
    }
    int activity =
        select(max_socket + 1, &fd_read_set, nullptr, nullptr, &timeout);

    // In case of a timeout, no message needs to be read.
    if (activity < 0) continue;

    // For each client, check if their fd is what trigerred select().
    for (auto& client : clients_) {
      if (!client.IsConnected()) continue;
      if (!FD_ISSET(client.socket, &fd_read_set)) continue;
      int len = read(client.socket, buffer, kBufferSize);
      if (len <= 0) {
        HandleClientDisconnect(client);
      } else {
        std::string message(buffer, len);

        // If the message received cannot be unmarshaled into a valid news
        // item then terminate connection with its client else process the
        // news item.
        common::NewsItem item;
        if (!common::NewsItem::FromString(message, item))
          HandleClientDisconnect(client);
        else
          send_acks_to.push(&client);
      }
    }
  }
  std::cout << "Thread has finished reading" << std::endl;
}

void Server::HandleClientDisconnect(ClientProxy& client) {
  std::cout << "Client Disconnectied: " << client.socket
            << " with IP: " << inet_ntoa(client.address.sin_addr)
            << " and port: " << ntohs(client.address.sin_port) << std::endl;
  close(client.socket);
  client.socket = 0;
}

void Server::SendAcksToClient(std::queue<ClientProxy*>& clients_to_ack) {
  while (clients_to_ack.size()) {
    ClientProxy* client = clients_to_ack.front();
    clients_to_ack.pop();
    if (client->IsConnected()) send(client->socket, "ACK", 3, 0);
  }
}
}  // namespace server