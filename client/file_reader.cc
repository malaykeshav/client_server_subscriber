#include "file_reader.h"

#include <iostream>
#include <string>

namespace client {

FileReader::FileReader(const std::string& file_name)
    : file_name_(file_name), file_(file_name, std::ios::binary) {}

bool FileReader::GetNextLine(common::NewsItem& item) {
    if (!file_.is_open())
        return "";
    std::string line;
    if (!std::getline(file_, line)) {
        file_.close();
        return false;
    }
    return common::NewsItem::FromString(line, item);
}

}  // namespace client