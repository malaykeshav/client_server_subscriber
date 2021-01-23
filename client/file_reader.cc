#include "file_reader.h"

namespace client {

FileReader::FileReader(const std::string& file_name)
    : file_name_(file_name), file_(file_name) {}

}  // namespace client