#ifndef SAMPLE_SERVER_H_
#define SAMPLE_SERVER_H_

#include <iostream>

#include "server/server.h"

int main(int argc, char *argv[]) {
    server::Server server;
    bool result = server.OpenSocket(9211);
    if (!result)
        return 0;
    server.StartListening();
    return 0;
}

#endif  // SAMPLE_SERVER_H_