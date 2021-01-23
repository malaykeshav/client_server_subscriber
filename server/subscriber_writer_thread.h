#ifndef SERVER_SUBSCRIBER_WRITER_THREAD_H_
#define SERVER_SUBSCRIBER_WRITER_THREAD_H_

#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace server {
class Subscriber;
// This thread manages publishing the news content on subscriber queues to
// disk at fixed time intervals.
class SubscriberWriterThread : public std::thread {
 public:
  explicit SubscriberWriterThread(std::vector<Subscriber*> subscribers,
                                  int time_interval_ms);
  // The run loop for the thread.
  void RunLoop();

  // Stops the thread and makes it joinable.
  void Stop();

  // Disable copy or move
  SubscriberWriterThread(const SubscriberWriterThread&) = delete;
  SubscriberWriterThread& operator=(const SubscriberWriterThread&) = delete;
  SubscriberWriterThread(SubscriberWriterThread&&) = delete;
  SubscriberWriterThread& operator=(SubscriberWriterThread&&) = delete;

 private:
  // Schedules the task on worker threads to flush subscriber queues.
  void ScheduleTasks();

  // List of subscribers to publish news items for.
  const std::vector<Subscriber*> subscribers_;
  
  // The time intervals at which this thread runs the task of flushing the
  // subscriber queue's news items.
  const int time_interval_ms_;

  bool should_stop_ = false;
};

}  // namespace server

#endif  // SERVER_SUBSCRIBER_WRITER_THREAD_H_
