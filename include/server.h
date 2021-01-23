#ifndef INCLUDE_SERVER_H_
#define INCLUDE_SERVER_H_

#include <netinet/in.h> 
#include <vector>

#include "../include/client_proxy.h"

class Server {
  public:
    Server() = default;
    ~Server() = default;

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
    void HandleNewClient(ClientProxy client);

    int socket_ = 0;
    sockaddr_in address_;
    std::vector<ClientProxy> clients_;
};


#endif  // INCLUDE_SERVER_H_
