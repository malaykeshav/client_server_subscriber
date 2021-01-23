#include "subscriber.h"

namespace server {

Subscriber::Subscriber(unsigned long long id) : id_(id) {}

void Subscriber::AddInterest(const std::string& interest) {
  interests_.insert(interest);
}

void Subscriber::PushNewsItem(const common::NewsItem& item) {
  const std::string& category = item.category;
  if (interests_.find(category) == interests_.end()) return;
  pending_items_.push(item);
}

void FlushNewsItemQueue() {
    
}

}  // namespace server
