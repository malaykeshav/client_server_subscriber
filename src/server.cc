#include "../include/server.h"

#include <arpa/inet.h>
#include <iostream>
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 

namespace {
constexpr int kListenPendingQueue = 3;
} // namespace

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
    fd_set fdset;
    FD_ZERO(&fdset);
    FD_SET(socket_, &fdset);
    while(true) {   
        // Listen for client connections. 
        int activity = 
            select(socket_ + 1 , &fdset , nullptr , nullptr , nullptr);   
       
        if (activity < 0) {   
            std::cerr << "Select Error" << std::endl;   
            return;
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
}

void Server::HandleNewClient(ClientProxy client) {
    clients_.push_back(client);
    
    std::string hello = "Server: Connected to server";
    send(client.socket, hello.c_str(), hello.size(), 0); 
}
