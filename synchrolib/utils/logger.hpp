#pragma once
#include <synchrolib/utils/general.hpp>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>

namespace synchrolib {

class Logger : public NonMovable, public NonCopyable {
public:
  enum class LogLevel {
    ERROR = 0,
    WARNING = 1,
    INFO = 2,
    VERBOSE = 3,
    DEBUG = 4
  };

  class LoggerStream : public NonMovable, public NonCopyable {
  public:
    template <typename T>
    LoggerStream& operator<<(const T& value) {
      if (Logger::log_level_ >= log_level_) {
        *Logger::log_stream_ << value;
      }
      return *this;
    }

    LoggerStream(Logger::LogLevel log_level, std::string name = "")
        : log_level_(log_level) {
      if (name.empty() && !Logger::log_name_stack_.empty()) {
        name = Logger::log_name_stack_.top();
      }

      *this << "[" << get_local_time() << "] ["
            << Logger::log_level_to_string(log_level_)
            << (name.empty() ? std::string() : std::string("@") + name) << "] ";
    }
    ~LoggerStream() { *this << "\n"; }

  private:
    Logger::LogLevel log_level_;
  };

  static LoggerStream error() { return LoggerStream(LogLevel::ERROR); }
  static LoggerStream warning() { return LoggerStream(LogLevel::WARNING); }
  static LoggerStream info() { return LoggerStream(LogLevel::INFO); }
  static LoggerStream verbose() { return LoggerStream(LogLevel::VERBOSE); }
  static LoggerStream debug() { return LoggerStream(LogLevel::DEBUG); }

  static void set_log_level(LogLevel level) { log_level_ = level; }
  static LogLevel get_log_level() { return log_level_; }

  static void set_log_stream(std::ostream* stream) { log_stream_ = stream; }

  class LogNameGuard : public NonCopyable, public NonMovable {
  public:
    LogNameGuard(std::string name) { Logger::log_name_stack_.push(name); }

    ~LogNameGuard() { Logger::log_name_stack_.pop(); }
  };
  static LogNameGuard set_log_name(std::string name) {
    return LogNameGuard(name);
  }

private:
  inline static std::ostream* log_stream_ = &std::cout;
  inline static LogLevel log_level_ = Logger::LogLevel::INFO;
  inline static std::stack<std::string> log_name_stack_ = std::stack<std::string>();

  Logger() {}

  static std::string log_level_to_string(LogLevel level) {
    switch (level) {
      case LogLevel::ERROR:
        return "ERROR";
      case LogLevel::WARNING:
        return "WARNING";
      case LogLevel::INFO:
        return "INFO";
      case LogLevel::VERBOSE:
        return "VERBOSE";
      case LogLevel::DEBUG:
        return "DEBUG";
      default:
        return "UNKNOWN";
    }
  }

  static std::string get_local_time() {
    auto now = SystemClock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  now.time_since_epoch()) %
        1000;
    auto timer = SystemClock::to_time_t(now);
    std::tm bt = *std::localtime(&timer);
    std::ostringstream oss;
    oss << std::put_time(&bt, "%H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
  }
};

}  // namespace synchrolib
