#include "file_reader.h"

#include <iostream>
#include <string>

namespace common {

FileReader::FileReader(const std::string& file_name)
    : file_name_(file_name), file_(file_name, std::ios::binary) {}

bool FileReader::GetNextLine(common::NewsItem& item) {
    if (!file_.is_open())
        return false;
    std::string line;
    if (!std::getline(file_, line)) {
        file_.close();
        return false;
    }
    return common::NewsItem::FromString(line, item);
}

bool FileReader::GetNextLineAsSubscriberItem(SubscriberItem& result) {
    if (!file_.is_open())
        return false;
    std::string line;
    if (!std::getline(file_, line)) {
        file_.close();
        return false;
    }
    
    result.subscriber_id = std::stoull(line, nullptr, 16);
    
    int begin = line.find_first_of('\"');
    int end = line.find_last_of('\"');
    result.category = line.substr(begin + 1, end - begin - 1);
    return true;
}
}  // namespace common