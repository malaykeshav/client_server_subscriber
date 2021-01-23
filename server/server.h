#ifndef SERVER_SERVER_H_
#define SERVER_SERVER_H_

#include <netinet/in.h>

#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>
#include <vector>

#include "../common/file_reader.h"
#include "client_proxy.h"
#include "subscriber.h"

namespace server {
class Server {
 public:
  Server(const std::string& config_file_name);
  ~Server();

  // Opens a socket to listen on. Returns true on success.
  bool OpenSocket(int port);

  // Starts listening for incoming connection from clients.
  void StartListening();

  // Disable copy or move
  Server(const Server&) = delete;
  Server& operator=(const Server&) = delete;
  Server(Server&&) = delete;
  Server& operator=(Server&&) = delete;

 private:
  // Reads the server config file to initialize the hash map of subscriber.
  void InitializeSubscriberConfig();

  // Initializes the thread that is responsible for reading the incoming
  // messages from the client.
  void InitializeReaderThread();
  void TerminateReaderThread();

  // An infinite loop that listes to client fds for changes. This is run on
  // a separate thread.
  void ReadFromClientLoop();

  void HandleNewClient(ClientProxy client);
  void HandleClientDisconnect(ClientProxy& client);

  // Sends an ACK to the list of clients in |clients_to_ack|. These clients
  // have sent a message and are waiting on ACK to send the next one.
  void SendAcksToClient(std::queue<ClientProxy*>& clients_to_ack);

  common::FileReader config_file_reader_;

  typedef unsigned long long SubscriberID;
  std::unordered_map<SubscriberID, std::unique_ptr<Subscriber>> subscribers_;

  int socket_ = 0;
  sockaddr_in address_;

  std::mutex clients_mtx_;
  std::vector<ClientProxy> clients_;
  std::queue<ClientProxy> clients_wait_queue_;

  std::unique_ptr<std::thread> reader_thread_;
  bool listening_ = false;
};

}  // namespace server

#endif  // SERVER_SERVER_H_
