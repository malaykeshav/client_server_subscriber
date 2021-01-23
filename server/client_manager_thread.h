#ifndef SERVER_CLIENT_MANAGER_THREAD_H_
#define SERVER_CLIENT_MANAGER_THREAD_H_

#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

#include "client_proxy.h"
#include "server_subscriber_delegate.h"

namespace server {

// Manages the lifetime of connection with clients assigned to this class.
// Server can add new clients for this to manage.
class ClientManagerThread : public std::thread {
 public:
  explicit ClientManagerThread(ServerSubscriberDelegate* delegate);

  const std::size_t size() const { return size_; }  

  // Adds a new client that is managed by this thread.
  void AddClient(const ClientProxy& client);

  // Main run loop that listes to client fds for changes. This is run on
  // a separate thread.
  void RunLoop();

  // Stop the run loop and make the thread joinable.
  void Stop() { should_run_ = false; }

 private:
  // Disconnects the client and performs any cleanup operation necessary.
  void HandleClientDisconnect(ClientProxy& client);

  // Returns true if the message is successfully handled.
  bool HandleClientMessage(ClientProxy& client, const std::string& message);

  // Sends an ACK to the list of clients in |clients_to_ack|. These clients
  // have sent a message and are waiting on ACK to send the next one.
  void SendAcksToClient(std::queue<ClientProxy*>& clients_to_ack);

  // Adds the client in the wait queue to list of clients that are
  // actively being listened to.
  void AddWaitingClients();

  // A delegate to send back processed messages to the server.
  ServerSubscriberDelegate* delegate_ = nullptr;

  // Mutex to modify the wait queue.
  std::mutex clients_mtx_;
  std::queue<ClientProxy> clients_wait_queue_;

  // Current list of managed clients.
  // Sparsely populated: Some entries of clients are disconnected.
  std::vector<ClientProxy> clients_;

  // Current number of clients being managed. May change after this is
  // called.
  std::size_t size_ = 0;

  // If set to false, the Run loop stops and the thread becomes
  // joinable.
  bool should_run_ = true;
};

}  // namespace server

#endif  // SERVER_CLIENT_MANAGER_THREAD_H_
