#ifndef INCLUDE_CLIENT_PROXY_H_
#define INCLUDE_CLIENT_PROXY_H_

#include <netinet/in.h> 
#include <string>

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


#endif  // INCLUDE_CLIENT_PROXY_H_
