#ifndef SERVER_SERVER_SUBSCRIBER_DELEGATE_H_
#define SERVER_SERVER_SUBSCRIBER_DELEGATE_H_

#include <vector>

namespace common {
class NewsItem;
}

namespace server {
class Subscriber;
class ServerSubscriberDelegate {
 public:
  // Calling this method pushes the list of items to the subscribers managed by
  // the server.
  virtual void PushItemsToSubscribers(
      const std::vector<common::NewsItem>& items) = 0;
};

}  // namespace server

#endif  // SERVER_SERVER_SUBSCRIBER_DELEGATE_H_