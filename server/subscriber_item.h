#ifndef SERVER_SUBSCRIBER_ITEM_H_
#define SERVER_SUBSCRIBER_ITEM_H_

#include <string>

namespace server {

struct SubscriberItem {
  std::string category;
  std::string title;
  int priority;

  std::string ToString() const;
  static bool FromString(const std::string& string, SubscriberItem& item);
};

}  // namespace common

#endif  // SERVER_SUBSCRIBER_ITEM_H_
