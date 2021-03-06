#ifndef SERVER_SUBSCRIBER_H_
#define SERVER_SUBSCRIBER_H_

#include <mutex>
#include <queue>
#include <string>
#include <unordered_set>

#include "../common/news_item.h"

namespace server {

class Subscriber {
 public:
  Subscriber(unsigned long long id, const std::string& out_folder);
  ~Subscriber() = default;

  // Add |interest| to list of things this subscriber is interested in.
  void AddInterest(const std::string& interest);

  // If the Subscriber is interested in the news item, it stores it in queue,
  // until the next flugh.
  void PushNewsItems(const std::vector<common::NewsItem>& items);

  // Flushes the news item in queue to disk.
  void FlushNewsItemQueue();

  // Disable copy or move
  Subscriber(const Subscriber&) = delete;
  Subscriber& operator=(const Subscriber&) = delete;
  Subscriber(Subscriber&&) = delete;
  Subscriber& operator=(Subscriber&&) = delete;

 private:
  unsigned long long id_ = -1;

  // Path to out folder where the subscriber will write to disk.
  const std::string out_folder_;

  // The intersts of this subscriber.
  std::unordered_set<std::string> interests_;

  // A mutex to read the queue with pending items.
  std::mutex pending_item_mtx_;

  // The news items waiting to be flushed to the subscriber.
  typedef std::priority_queue<common::NewsItem, std::vector<common::NewsItem>,
                      common::NewsItem::OrderByPriority> NewsItemPriorityQueue;
  NewsItemPriorityQueue pending_items_;
};

}  // namespace server

#endif  // SERVER_SUBSCRIBER_H_
