#include "news_item.h"
#include <string>

namespace common {

std::string NewsItem::ToString() const {
    std::string ret = "";
    ret = "\"" + category + "\":\"" + title + "\":" + std::to_string(priority);
    return ret;
}

bool NewsItem::FromString(const std::string& line, NewsItem& item) {
    int i = 0;

    while (line[i] != '\"' && i < line.size())
        i++;
    i++;

    if (i >= line.size())
        return false;

    item.category = "";
    while (line[i] != '\"' && line[i-1] != '\\' && i < line.size()) {
        item.category.push_back(line[i]);
        i++;
    }

    if (i >= line.size())
        return false;
    i++; 
    while(line[i] != '\"' && i < line.size())
        i++;
    
    if (i >= line.size())
        return false;
    i++;

    item.title = "";
    while (line[i] != '\"' && line[i-1] != '\\' && i < line.size()) {
        item.title.push_back(line[i]);
        i++;
    }

    while (i < line.size() && (line[i] < '0' || line[i] > '9'))
        i++;
    
    if (i >= line.size())
        return false;
    
    item.priority = 0;
    while(line[i] >= '0' && line[i] <= '9') {
        item.priority = item.priority * 10 + (line[i] - '0');
        i++;
    }
    return true;
}

} // namespace common