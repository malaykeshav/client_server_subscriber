#ifndef INCLUDE_CLIENT_PROXY_H_
#define INCLUDE_CLIENT_PROXY_H_

#include <netinet/in.h> 

struct ClientProxy {
    bool IsValid() const { return socket; }
    int socket = 0;
    struct sockaddr_in address;
};


#endif  // INCLUDE_CLIENT_PROXY_H_
