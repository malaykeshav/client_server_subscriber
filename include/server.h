#ifndef INCLUDE_SERVER_H_
#define INCLUDE_SERVER_H_

#include <netinet/in.h> 
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "../include/client_proxy.h"

class Server {
  public:
    Server();
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
    void HandleNewClient(ClientProxy client);

    void InitializeReaderThread();
    void TerminateReaderThread();
    void ReadFromClientLoop();
    void HandleClientDisconnect(ClientProxy& client);

    int socket_ = 0;
    sockaddr_in address_;

    std::mutex clients_mtx_;
    std::vector<ClientProxy> clients_;
    std::queue<ClientProxy> clients_wait_queue_;

    std::unique_ptr<std::thread> reader_thread_;
    bool listening_ = false;
};


#endif  // INCLUDE_SERVER_H_
