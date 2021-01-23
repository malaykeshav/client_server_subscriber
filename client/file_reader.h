#ifndef CLIENT_FILE_READER_H_
#define CLIENT_FILE_READER_H_

#include <netinet/in.h>

#include <fstream>
#include <string>

namespace client {

class FileReader {
 public:
  FileReader(const std::string& file_name);
  ~FileReader() = default;

  // Disable copy or move
  FileReader(const FileReader&) = delete;
  FileReader& operator=(const FileReader&) = delete;
  FileReader(FileReader&&) = delete;
  FileReader& operator=(FileReader&&) = delete;

 private:
  const std::string file_name_;
  std::ifstream file_;
};

}  // namespace client

#endif  // CLIENT_FILE_READER_H_
