#include "../include/server.h"

#include <arpa/inet.h>
#include <iostream>
#include <memory>
#include <mutex>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h> 

namespace {
constexpr int kListenPendingQueue = 3;
constexpr int kReadTimeOutSec = 1;
constexpr int kBufferSize = 1024;

void InsertClient(std::vector<ClientProxy>& clients,
                  const ClientProxy& client) {
    for (ClientProxy& c : clients) {
        if (c.IsConnected())
            continue;
        c.socket = client.socket;
        c.address = client.address;
        return;
    }
    clients.push_back(client);
}
} // namespace

Server::Server() {
}

Server::~Server() {
    listening_ = false;
    reader_thread_->join();
}

bool Server::OpenSocket(int port) {
    int opt = 1; 
    socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_ == 0) { 
        std::cerr << "Failed to create socket" << std::endl; 
        return false; 
    } 
       
    if (setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
        &opt, sizeof(opt))) { 
        std::cerr << "Failed to setsockopt" << std::endl;
        return false;
    } 

    address_.sin_family = AF_INET; 
    address_.sin_addr.s_addr = INADDR_ANY; 
    address_.sin_port = htons(port); 
       
    // Forcefully attaching socket to the port 8080 
    if (bind(socket_, (struct sockaddr *)&address_, 
        sizeof(address_))<0) { 
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
    FD_ZERO(&fdset);
    FD_SET(socket_, &fdset);
    while(true) {   
        // Listen for client connections. 
        int activity = 
            select(socket_ + 1 , &fdset , nullptr , nullptr , nullptr);   
       
        if (activity < 0) {   
            std::cerr << "Select Error" << std::endl;   
            break;
        }   
             
        //If its not the server socket, then ignore.
        if (!FD_ISSET(socket_, &fdset)) {
            std::cerr << "Message received on non server socket" << std::endl;
            continue;  
        }
        ClientProxy client;
        int addrlen = sizeof(client.address);

        client.socket = accept(socket_, (struct sockaddr *)&client.address, 
                (socklen_t*)&addrlen);

        if (client.socket < 0) {   
            std::cerr << "Failed to accept client" << std::endl;   
            continue;  
        }   

        std::cout << "New connection: " << client.socket 
                  << " with IP: " << inet_ntoa(client.address.sin_addr)
                  << " and port: " << ntohs(client.address.sin_port) 
                  << std::endl;

        HandleNewClient(client);
    }
    listening_ = false;
    TerminateReaderThread();
}

void Server::TerminateReaderThread() {
    reader_thread_->join();
    reader_thread_.reset(nullptr);
}

void Server::HandleNewClient(ClientProxy client) {
    {
        std::unique_lock<std::mutex> lck (clients_mtx_);
        clients_wait_queue_.push(client);
    }
    
    std::string hello = "Server: Connected to server";
    send(client.socket, hello.c_str(), hello.size(), 0); 
}

void Server::InitializeReaderThread() {
    // Initialize a thread that will read responses from clients.
    listening_ = true;
    reader_thread_ = 
        std::make_unique<std::thread>(&Server::ReadFromClientLoop, this);
}

void Server::ReadFromClientLoop() {
    std::cout << "Thread is beginning to read in loop" << std::endl;
    fd_set fd_read_set;
    FD_ZERO(&fd_read_set);
    
    struct timeval timeout;        
    timeout.tv_sec = kReadTimeOutSec;             
    timeout.tv_usec = 0;

    char buffer[kBufferSize] = {0};

    while(listening_) {
        int max_socket = 0;
        {
            // Check if any new client has connected since last iteration.
            std::unique_lock<std::mutex> lck (clients_mtx_);
            while(clients_wait_queue_.size()) {
                const ClientProxy client = clients_wait_queue_.front();
                clients_wait_queue_.pop();
                if (!client.IsConnected()) 
                    continue;
                InsertClient(clients_, client);
                std::cout << "Reader thread is listening to new client: "
                          << client.socket << std::endl;
            }

            FD_ZERO(&fd_read_set);
            for (const auto& client : clients_) {
                if (!client.IsConnected())
                    continue;
                FD_SET(client.socket, &fd_read_set); 
                max_socket = std::max(max_socket, client.socket);
            }
        }
        int activity = 
            select(max_socket + 1, &fd_read_set, nullptr, nullptr, &timeout);
        if (activity < 0)
            continue;
        
        for (auto& client : clients_) {
            if (client.IsConnected()) {
                if (!FD_ISSET(client.socket, &fd_read_set))
                    continue;
                int len = read(client.socket, buffer, kBufferSize);
                if (len <= 0) {
                    HandleClientDisconnect(client);
                } else {
                    std::string message(buffer, len);
                    std::cout << "Message from client: " << message << std::endl;
                }
            }
        }
    }

    std::cout << "Thread has finished reading" << std::endl;
}

void Server::HandleClientDisconnect(ClientProxy& client) {
    std::cout << "Client Disconnectied: " << client.socket 
              << " with IP: " << inet_ntoa(client.address.sin_addr)
              << " and port: " << ntohs(client.address.sin_port) 
              << std::endl;
    close(client.socket);
    client.socket = 0;
}
