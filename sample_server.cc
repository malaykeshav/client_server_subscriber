#ifndef SAMPLE_SERVER_H_
#define SAMPLE_SERVER_H_

#include <iostream>

#include "server/server.h"

int main(int argc, char *argv[]) {
  if (argc < 1) {
    std::cerr << "Insufficient arguments provided." << std::endl;
    return 0;
  }
  std::string config_file_name = argv[1];

  server::Server server(config_file_name);
  bool result = server.OpenSocket(9211);
  if (!result) return 0;
  server.StartListening();
  return 0;
}

#endif  // SAMPLE_SERVER_H_