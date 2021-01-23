#ifndef SERVER_CLIENT_PROXY_H_
#define SERVER_CLIENT_PROXY_H_

#include <netinet/in.h>

#include <string>

namespace server {
struct ClientProxy {
  bool IsValid() const { return socket; }

  friend bool operator<(const ClientProxy& a, const ClientProxy& b) {
    return a.socket < b.socket;
  }

  friend bool operator==(const ClientProxy& a, const ClientProxy& b) {
    return a.socket == b.socket;
  }

  bool IsConnected() const { return socket > 0; }

  int socket = 0;
  struct sockaddr_in address;
};

}  // namespace server

#endif  // SERVER_CLIENT_PROXY_H_
