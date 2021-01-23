#ifndef COMMON_FILE_READER_H_
#define COMMON_FILE_READER_H_

#include "news_item.h"

#include <fstream>
#include <string>

namespace common {

struct SubscriberItem {
  unsigned long long subscriber_id;
  std::string category;
};

class FileReader {
 public:
  FileReader(const std::string& file_name);
  ~FileReader() = default;

  // Fills the |item| object with the next line of data from the file. Returns
  // true if |item| is a valid entry, else returns false.
  bool GetNextLine(common::NewsItem& item);

  // Fills the |result| string with the next line of data from the file. Returns
  // true if |result| is filled else returns fals on failure.
  bool GetNextLineAsSubscriberItem(SubscriberItem& result);

  // Disable copy or move
  FileReader(const FileReader&) = delete;
  FileReader& operator=(const FileReader&) = delete;
  FileReader(FileReader&&) = delete;
  FileReader& operator=(FileReader&&) = delete;

 private:

  const std::string file_name_;
  std::ifstream file_;
  int lines_read_ = 0;
};

}  // namespace common

#endif  // COMMON_FILE_READER_H_
