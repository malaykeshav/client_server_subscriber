#include "subscriber.h"

#include <fstream>
#include <iostream>
#include <mutex>
#include <string>

namespace server {
namespace {
std::string GetFullPath(const std::string& base_path, unsigned long long id) {
  std::string file_name = "subscriber_" + std::to_string(id) + ".txt";
  std::string full_path = base_path;
  if (full_path.back() != '/') full_path.push_back('/');
  full_path += file_name;
  return full_path;
}
}  // namespace

Subscriber::Subscriber(unsigned long long id, const std::string& out_folder)
    : id_(id), out_folder_(out_folder) {}

void Subscriber::AddInterest(const std::string& interest) {
  interests_.insert(interest);
}

void Subscriber::PushNewsItems(const std::vector<common::NewsItem>& items) {
  std::unique_lock<std::mutex> lock(pending_item_mtx_);
  for (const auto& item : items) {
    const std::string& category = item.category;
    if (interests_.find(category) == interests_.end()) continue;
    pending_items_.push(item);
  }
}

void Subscriber::FlushNewsItemQueue() {
  NewsItemPriorityQueue items_queue;
  {
    std::unique_lock<std::mutex> lock(pending_item_mtx_);
    pending_items_.swap(items_queue);
  }

  if (!items_queue.size()) return;

  std::ofstream out(GetFullPath(out_folder_, id_), std::ios::app);
  if (!out.is_open()) return;

  while (items_queue.size()) {
    const auto item = items_queue.top();
    items_queue.pop();
    out << item.ToString() << std::endl;
  }
  out << "END OF BATCH" << std::endl;
  out.close();
}

}  // namespace server
