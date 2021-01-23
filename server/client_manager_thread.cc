#include "client_manager_thread.h"

#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include <iostream>

#include "../common/news_item.h"

namespace server {
namespace {
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
}  // namespace

ClientManagerThread::ClientManagerThread()
    : std::thread(&ClientManagerThread::RunLoop, this) {}

void ClientManagerThread::AddClient(const ClientProxy& client) {
  std::unique_lock<std::mutex> lck(clients_mtx_);
  clients_wait_queue_.push(client);
  size_++;
}

// Main run loop that listes to client fds for changes. This is run on
// a separate thread.
void ClientManagerThread::RunLoop() {
  std::cout << "Thread is beginning to read in loop" << std::endl;
  fd_set fd_read_set;
  FD_ZERO(&fd_read_set);

  struct timeval timeout;
  timeout.tv_sec = kReadTimeOutSec;
  timeout.tv_usec = 0;

  char buffer[kBufferSize] = {0};

  std::queue<ClientProxy*> send_acks_to;

  while (should_run_) {
    // Send "ACK" to clients so they send the next message.
    SendAcksToClient(send_acks_to);
    AddWaitingClients();

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
        if (HandleClientMessage(client, message)) send_acks_to.push(&client);
      }
    }
  }
  std::cout << "Thread has finished reading" << std::endl;
}

void ClientManagerThread::HandleClientDisconnect(ClientProxy& client) {
  std::cout << "Client Disconnectied: " << client.socket
            << " with IP: " << inet_ntoa(client.address.sin_addr)
            << " and port: " << ntohs(client.address.sin_port) << std::endl;
  close(client.socket);
  client.socket = 0;

  size_--;
}

bool ClientManagerThread::HandleClientMessage(ClientProxy& client,
                                              const std::string& message) {
  // If the message received cannot be unmarshaled into a valid news
  // item then terminate connection with its client else process the
  // news item.
  common::NewsItem item;
  if (!common::NewsItem::FromString(message, item)) {
    HandleClientDisconnect(client);
    return false;
  }
  std::cout << "Message received: " << message << std::endl;
  return true;
}

void ClientManagerThread::SendAcksToClient(
    std::queue<ClientProxy*>& clients_to_ack) {
  while (clients_to_ack.size()) {
    ClientProxy* client = clients_to_ack.front();
    clients_to_ack.pop();
    if (client->IsConnected()) send(client->socket, "ACK", 3, 0);
  }
}

void ClientManagerThread::AddWaitingClients() {
  // Check if any new client has connected since last iteration and start
  // listening to them as well.
  std::unique_lock<std::mutex> lck(clients_mtx_);

  while (clients_wait_queue_.size()) {
    const ClientProxy client = clients_wait_queue_.front();
    clients_wait_queue_.pop();
    if (!client.IsConnected()) continue;
    InsertClient(clients_, client);
    std::cout << "Reader thread is listening to new client: " << client.socket
              << std::endl;
  }
}
}  // namespace server