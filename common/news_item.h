#ifndef COMMON_NEWS_ITEM_H_
#define COMMON_NEWS_ITEM_H_

#include <string>

namespace common {

struct NewsItem {
  std::string category;
  std::string title;
  int priority;

  std::string ToString() const;
  static bool FromString(const std::string& string, NewsItem& item);

  struct OrderByPriority {
      bool operator()(const NewsItem& a, const NewsItem& b) {
          return a.priority < b.priority;
      }
  };
};

}  // namespace common

#endif  // COMMON_NEWS_ITEM_H_
