#ifndef SAMPLE_CLIENT_H_
#define SAMPLE_CLIENT_H_

#include <iostream>

#include "client/client.h"

int main(int argc, char *argv[]) {
    if (!argc) {
        std::cerr << "Need input file to start client." << std::endl;
        return 0;
    }
    std::string file_name = argv[1];
    client::Client client(file_name);
    bool result = client.ConnectToServer("127.0.0.1", 9211);
    if (!result)
        return 0;
    client.RunLoop();
    return 0;
}

#endif  // SAMPLE_CLIENT_H_