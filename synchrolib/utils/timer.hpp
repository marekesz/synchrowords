#pragma once
#include <synchrolib/utils/general.hpp>
#include <synchrolib/utils/logger.hpp>
#include <chrono>
#include <iostream>

namespace synchrolib {

class Timer : public NonMovable, public NonCopyable {  // TODO: movable
public:
  Timer(std::string name)
      : name_(name), start_(time_since_epoch()), stopped_(false) {}
  ~Timer() { stop(); }

  template<bool Log=true, typename Unit=std::chrono::milliseconds>
  size_t stop() {
    if (stopped_) {
      return 0;
    }

    stopped_ = true;
    auto now = time_since_epoch();
    auto cnt = std::chrono::duration_cast<Unit>(now - start_);

    if constexpr (Log) {
      if (Logger::get_log_level() == Logger::LogLevel::DEBUG) {
        std::cerr << "[" << name_ << "] " << cnt.count() << "ms" << std::endl;
        // TODO: fix when g++ supports << cnt
      }
    }
    return cnt.count();
  }

private:
  std::string name_;
  Clock::duration start_;
  bool stopped_;

  Clock::duration time_since_epoch() {
    return Clock::now().time_since_epoch();
  }
};

}  // namespace synchrolib
