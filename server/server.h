#ifndef SERVER_SERVER_H_
#define SERVER_SERVER_H_

#include <netinet/in.h>

#include <memory>
#include <mutex>
#include <queue>
#include <unordered_map>
#include <vector>

#include "../common/file_reader.h"
#include "client_proxy.h"
#include "server_subscriber_delegate.h"


namespace server {
class ClientManagerThread;
class Subscriber;
class SubscriberWriterThread;
class Server : public ServerSubscriberDelegate {
 public:
  explicit Server(const std::string& config_file_name,
                  const std::string& subscriber_out_folder);
  ~Server();

  // Opens a socket to listen on. Returns true on success.
  bool OpenSocket(int port);

  // Starts listening for incoming connection from clients.
  void StartListening();

  // overrides ServerSubscriberDelegate:
  void PushItemsToSubscribers(
      const std::vector<common::NewsItem>& items) override;

  // Disable copy or move
  Server(const Server&) = delete;
  Server& operator=(const Server&) = delete;
  Server(Server&&) = delete;
  Server& operator=(Server&&) = delete;

 private:
  // Reads the server config file to initialize the hash map of subscriber.
  void InitializeSubscriberConfig(const std::string& subscriber_out_path);

  // Initializes the writer thread to manage flushing item to disk for each
  // subscriber.
  void InitializeWriterThread();
  void TerminateWriterThread();

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

  // A thread responsible for managing the flush of news items pending for each
  // subscriber.
  std::unique_ptr<SubscriberWriterThread> subscriber_writer_thread_;
};

}  // namespace server

#endif  // SERVER_SERVER_H_
