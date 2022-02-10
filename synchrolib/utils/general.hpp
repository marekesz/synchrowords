#pragma once
#include <chrono>
#include <cstdint>
#include <algorithm>

namespace synchrolib {

using uint = unsigned int;
using int64 = std::int_fast64_t;
using uint64 = std::uint_fast64_t;

using int8l = std::int_least8_t;
using int16l = std::int_least16_t;
using int32l = std::int_least32_t;
using int64l = std::int_least64_t;
using uint8l = std::uint_least8_t;
using uint16l = std::uint_least16_t;
using uint32l = std::uint_least32_t;
using uint64l = std::uint_least64_t;

using Clock = std::chrono::steady_clock;
using SystemClock = std::chrono::system_clock;

class NonCopyable {
public:
  NonCopyable(const NonCopyable&) = delete;
  NonCopyable& operator=(const NonCopyable&) = delete;
  NonCopyable() {}
};

class NonMovable {
public:
  NonMovable(NonMovable&&) = delete;
  NonMovable& operator=(NonMovable&&) = delete;
  NonMovable() {}
};

}  // namespace synchrolib
