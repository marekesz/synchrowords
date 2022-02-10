#pragma once
#include <iomanip>
#include <sstream>
#include <string>
#include <type_traits>

namespace synchrolib {

inline std::string get_megabytes(size_t mem, int precision = 0) {
  std::stringstream ss;
  ss << std::fixed;
  ss << std::setprecision(precision);
  ss << static_cast<long double>(mem) / 1024 / 1024;
  ss << "MB";
  return ss.str();
}

template <typename T, typename A>
size_t get_memory_usage_impl(const FastVector<T, A>& vec) {
  return vec.capacity() * sizeof(T);
}

template <typename T, size_t N>
constexpr size_t get_memory_usage_impl(const std::array<T, N>& vec) {
  return N * sizeof(T);
}

struct MemoryUsage;

template <typename T>
size_t get_memory_usage(const T& t) {
  if constexpr (std::is_base_of<MemoryUsage, T>::value) {
    return t.get_memory_usage();
  } else {
    return get_memory_usage_impl(t);
  }
}

struct MemoryUsage {
  virtual size_t get_memory_usage() const = 0;

  // memory usage with additional elements
  template <typename T>
  size_t get_memory_usage_with_additional_objects(const T& t) {
    return get_memory_usage() + synchrolib::get_memory_usage(t);
  }
  template <typename T, typename... Args>
  size_t get_memory_usage_with_additional_objects(const T& t, Args&&... args) {
    return synchrolib::get_memory_usage(t) +
        get_memory_usage_with_additional_objects(std::forward<Args>(args)...);
  }
};

}  // namespace synchrolib