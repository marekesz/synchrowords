#pragma once
#include <synchrolib/utils/general.hpp>
#include <synchrolib/utils/logger.hpp>
#include <chrono>
#include <iostream>

namespace synchrolib {

class Timer : public NonMovable, public NonCopyable {  // TODO: movable
public:
  Timer(std::string name)
      : name_(name), start_(ms_since_epoch()), stopped_(false) {}
  ~Timer() { stop(); }

  template<bool Log=true>
  size_t stop() {
    if (stopped_) {
      return 0;
    }

    stopped_ = true;
    size_t now = ms_since_epoch();
    if constexpr (Log) {
      if (Logger::get_log_level() == Logger::LogLevel::DEBUG) {
        std::cerr << "[" << name_ << "] " << now - start_ << "ms" << std::endl;
      }
    }
    return now - start_;
  }

private:
  std::string name_;
  size_t start_;
  bool stopped_;

  size_t ms_since_epoch() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        Clock::now().time_since_epoch())
        .count();
  }
};

}  // namespace synchrolib
