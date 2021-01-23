#ifndef SAMPLE_CLIENT_H_
#define SAMPLE_CLIENT_H_

#include <iostream>

#include "include/client.h"

int main(int argc, char *argv[]) {
    Client client;
    bool result = client.ConnectToServer("127.0.0.1", 9211);
    if (!result)
        return 0;
    client.RunLoop();
    return 0;
}

#endif  // SAMPLE_CLIENT_H_