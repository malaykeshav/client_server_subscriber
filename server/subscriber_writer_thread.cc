#include "subscriber_writer_thread.h"

#include <chrono>
#include <thread>

#include "subscriber.h"

namespace server {

SubscriberWriterThread::SubscriberWriterThread(
    std::vector<Subscriber*> subscribers, int time_interval_ms)
    : std::thread(&SubscriberWriterThread::RunLoop, this),
      subscribers_(subscribers),
      time_interval_ms_(time_interval_ms) {}

void SubscriberWriterThread::RunLoop() {
  while (!should_stop_) {
    std::this_thread::sleep_for(std::chrono::milliseconds(time_interval_ms_));
    ScheduleTasks();
  }
}

void SubscriberWriterThread::Stop() {
    should_stop_ = true;
}

void SubscriberWriterThread::ScheduleTasks() {
  for (Subscriber* subscriber : subscribers_) subscriber->FlushNewsItemQueue();
}
}  // namespace server