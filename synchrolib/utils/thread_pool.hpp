#pragma once
#include <synchrolib/utils/general.hpp>
#include <utility>
#include <thread>
#include <condition_variable>
#include <queue>
#include <mutex>

namespace synchrolib {

class ThreadPool : public NonCopyable, public NonMovable {
public:
  using Job = std::function<void()>;

  ThreadPool(): running(false) {}
  ~ThreadPool() {
    if (running) {
      wait();
    }
  }

  void start(size_t num_threads) {
    running = true;
    terminate = false;
    for (size_t i = 0; i < num_threads; ++i) {
      threads.push_back(std::thread([this] { this->loop(); }));
    }
  }

  void add_job(Job job) {
    {
      std::unique_lock<std::mutex> lock(queue_mutex);
      queue.push(job);
    }

    queue_cv.notify_one();
  }

  void wait() {
    {
      std::unique_lock<std::mutex> lock(queue_mutex);
      terminate = true;
    }

    queue_cv.notify_all();

    for (auto& thread : threads) {
      thread.join();
    }
    threads.clear();

    running = false;
  }

private:
  std::vector<std::thread> threads;
  bool terminate;
  bool running;

  std::mutex queue_mutex;
  std::condition_variable queue_cv;
  std::queue<Job> queue;

  void loop() {
    while (true) {
      Job job;
      {
        std::unique_lock<std::mutex> lock(queue_mutex);
        queue_cv.wait(lock, [this](){
          return !queue.empty() || terminate;
        });

        if (queue.empty()) {
          break;
        }

        job = queue.front();
        queue.pop();
      }

      job();
    }
  }
};

}  // namespace synchrolib