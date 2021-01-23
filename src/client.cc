#include "../include/client.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <iostream>
#include <stdio.h> 
#include <stdlib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>   

namespace {
constexpr int kConnectionTimeOutSec = 10;
} // namespace

bool Client::ConnectToServer(const std::string& ip_address, int port) {
    socket_ = socket(AF_INET, SOCK_STREAM, 0);

    // Make the socket non blocking.
    fcntl(socket_, F_SETFL, O_NONBLOCK);

    if (socket_ < 0) { 
        std::cerr << "Failed to create client socket" << std::endl;
        return false; 
    } 
   
    serv_addr_.sin_family = AF_INET; 
    serv_addr_.sin_port = htons(port); 
       
    if(inet_pton(AF_INET, ip_address.c_str(), &serv_addr_.sin_addr) <= 0) { 
        std::cerr << "Invalid IP address provided" << std::endl; 
        return false; 
    } 
   
    int result = 
        connect(socket_, (struct sockaddr *)&serv_addr_, sizeof(serv_addr_)); 
    ip_address_ = ip_address;
    return true;
}

void Client::RunLoop() {
    if (socket_ <= 0)
        return;
    fd_set fd_read_set;
    struct timeval tv;

    FD_ZERO(&fd_read_set);
    FD_SET(socket_, &fd_read_set);

    tv.tv_sec = kConnectionTimeOutSec;             
    tv.tv_usec = 0;
    
    while (true) {
        if (select(socket_ + 1, &fd_read_set, nullptr, nullptr, &tv) == 1) {
            int so_error;
            socklen_t len = sizeof so_error;

            if (FD_ISSET(socket_, &fd_read_set)) {
                std::string message = ReadFromSocket();
                
                // Check to see if the socket is still connected.
                if (should_disconnect_) {
                    std::cout << "Connection terminated by server" << std::endl;
                    close(socket_);
                    return;
                }
                std::cout << message << std::endl;
                SendToServer("Message sent from client");
            }
        }
    }
}

std::string Client::ReadFromSocket() {
    if (socket_ <= 0)
        return "";
    
    int len = read(socket_ , buffer_, 1024); 
    
    // This is the case that the connection has been disconnected.
    if (len <= 0) {
        should_disconnect_ = true;
        return "";
    }

    std::string ret(buffer_, len);
    return ret;
}

void Client::SendToServer(const std::string& message) {
    if (socket_ <= 0)
        return;
    send(socket_ , message.c_str() , message.size() , 0); 
}

