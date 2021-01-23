#ifndef SERVER_SUBSCRIBER_H_
#define SERVER_SUBSCRIBER_H_

#include <queue>
#include <string>
#include <unordered_set>

#include "../common/news_item.h"

namespace server {

class Subscriber {
 public:
  Subscriber(unsigned long long id);
  ~Subscriber() = default;

  // Add |interest| to list of things this subscriber is interested in.
  void AddInterest(const std::string& interest);

  // If the Subscriber is interested in the news item, it stores it in queue,
  // until the next flugh.
  void PushNewsItem(const common::NewsItem& item);

  // Flushes the news item in queue to disk.
  void FlushNewsItemQueue();

  // Disable copy or move
  Subscriber(const Subscriber&) = delete;
  Subscriber& operator=(const Subscriber&) = delete;
  Subscriber(Subscriber&&) = delete;
  Subscriber& operator=(Subscriber&&) = delete;

 private:
  unsigned long long id_ = -1;

  // The intersts of this subscriber.
  std::unordered_set<std::string> interests_;

  // The news items waiting to be flushed to the subscriber.
  std::priority_queue<common::NewsItem, std::vector<common::NewsItem>,
                      common::NewsItem::OrderByPriority>
      pending_items_;
};

}  // namespace server

#endif  // SERVER_SUBSCRIBER_H_
