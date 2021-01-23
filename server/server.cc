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
#include <vector>

#include "../common/news_item.h"
#include "client_manager_thread.h"
#include "subscriber.h"
#include "subscriber_writer_thread.h"

namespace server {
namespace {
constexpr int kListenPendingQueue = 3;
constexpr int kSubscriberFlushIntervalMs = 100;
}  // namespace

Server::Server(const std::string& config_file_name,
               const std::string& subscriber_out_path)
    : config_file_reader_(config_file_name) {
  InitializeSubscriberConfig(subscriber_out_path);
  InitializeWriterThread();
}

Server::~Server() {
  TerminateReaderThread();
  TerminateWriterThread();
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
  while (true) {
    FD_ZERO(&fdset);
    FD_SET(socket_, &fdset);
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

    HandleClientConnect(client);
  }
  TerminateReaderThread();
}

void Server::PushItemsToSubscribers(
    const std::vector<common::NewsItem>& items) {
  for (auto& it : subscribers_) it.second->PushNewsItems(items);
}

void Server::InitializeSubscriberConfig(
    const std::string& subscriber_out_path) {
  common::SubscriberItem item;

  while (config_file_reader_.GetNextLineAsSubscriberItem(item)) {
    auto it = subscribers_.find(item.subscriber_id);
    if (it == subscribers_.end()) {
      subscribers_.insert(
          {item.subscriber_id, std::make_unique<Subscriber>(
                                   item.subscriber_id, subscriber_out_path)});
    }
    auto& subscriber = subscribers_.at(item.subscriber_id);
    subscriber->AddInterest(item.category);
  }
}

void Server::TerminateReaderThread() {
  if (!reader_thread_) return;
  reader_thread_->Stop();
  reader_thread_->join();
  reader_thread_.reset(nullptr);
}

void Server::HandleClientConnect(ClientProxy client) {
  // POSSIBLE OPTIMIZATION
  // This could be made smarter by picking a thread that is least loaded.
  reader_thread_->AddClient(client);
}

void Server::InitializeReaderThread() {
  // Initialize a thread that will read responses from clients.
  reader_thread_ = std::make_unique<ClientManagerThread>(this);
}

void Server::InitializeWriterThread() {
  std::vector<Subscriber*> subscribers;
  for (const auto& it : subscribers_) subscribers.push_back(it.second.get());

  subscriber_writer_thread_ = std::make_unique<SubscriberWriterThread>(
      subscribers, kSubscriberFlushIntervalMs);
}

void Server::TerminateWriterThread() {
  if (!subscriber_writer_thread_) return;
  subscriber_writer_thread_->Stop();
  subscriber_writer_thread_->join();
  subscriber_writer_thread_.reset(nullptr);
}

}  // namespace server