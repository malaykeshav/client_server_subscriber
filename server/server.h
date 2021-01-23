#ifndef SERVER_SERVER_H_
#define SERVER_SERVER_H_

#include <netinet/in.h>

#include <memory>
#include <mutex>
#include <queue>
#include <unordered_map>
#include <vector>

#include "../common/file_reader.h"
#include "client_manager_thread.h"
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

  void HandleClientConnect(ClientProxy client);

  // Initializes the thread that is responsible for reading the incoming
  // messages from the client.
  void InitializeReaderThread();
  void TerminateReaderThread();

  common::FileReader config_file_reader_;

  typedef unsigned long long SubscriberID;
  std::unordered_map<SubscriberID, std::unique_ptr<Subscriber>> subscribers_;

  int socket_ = 0;
  sockaddr_in address_;

  // POSSIBLE OPTIMIZATION
  // This coule be improved by haveing a pool of threads that each listen
  // to a subset of clients.
  std::unique_ptr<ClientManagerThread> reader_thread_;
};

}  // namespace server

#endif  // SERVER_SERVER_H_
