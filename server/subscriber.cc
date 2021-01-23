#include "subscriber.h"

#include <iostream>

namespace server {

Subscriber::Subscriber(unsigned long long id) : id_(id) {}

void Subscriber::AddInterest(const std::string& interest) {
  interests_.insert(interest);
}

void Subscriber::PushNewsItems(const std::vector<common::NewsItem>& items) {
  for (const auto& item : items) {
    const std::string& category = item.category;
    if (interests_.find(category) == interests_.end()) continue;
    pending_items_.push(item);
  }
}

void FlushNewsItemQueue() {}

}  // namespace server
