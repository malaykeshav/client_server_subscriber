#ifndef CLIENT_CLIENT_H_
#define CLIENT_CLIENT_H_

#include "../common/file_reader.h"

#include <netinet/in.h>
#include <string>

namespace client {

class Client {
 public:
  Client(const std::string& input_file_name);
  ~Client() = default;

  // Connects the client to a server on specified ip address
  // and port.
  bool ConnectToServer(const std::string& ip_address, int port);

  void RunLoop();

  // Reads from the socket and returns a string.
  // If socket is invalid, returns an empty string.
  std::string ReadFromSocket();

  // Sends a message to the server connected via |socket_|.
  // If socket is invalid, no message is sent.
  void SendToServer(const std::string& message);

  // Disable copy or move
  Client(const Client&) = delete;
  Client& operator=(const Client&) = delete;
  Client(Client&&) = delete;
  Client& operator=(Client&&) = delete;

 private:
  int socket_ = 0;
  struct sockaddr_in serv_addr_;
  std::string ip_address_;
  char buffer_[1024];
  bool should_disconnect_ = false;
  common::FileReader file_reader_;
};

}  // namespace client

#endif  // CLIENT_CLIENT_H_
