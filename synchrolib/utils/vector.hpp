#pragma once
#include <external/uwr/container/vector.hpp>
#include <future>
// #include <vector>

namespace synchrolib {

template<typename T, typename A=uwr::mem::hybrid_allocator_bs<T>>
using FastVector = uwr::vector<T, A>;
// template<typename T, typename A=std::allocator<T>>
// using FastVector = std::vector<T, A>;

template<typename T, typename A>
void keep_unique(FastVector<T, A>& vec) {
  vec.resize(std::distance(vec.begin(), std::unique(vec.begin(), vec.end())));
}
template<typename T, typename A>
void sort_keep_unique(FastVector<T, A>& vec) {
  std::sort(vec.begin(), vec.end());
  keep_unique(vec);
}

template<class T>
void parallel_sort(T* data, int len, int grainsize) {
  if (len < grainsize) {
    std::sort(data, data + len, std::less<T>());
  } else {
    auto future = std::async(parallel_sort<T>, data, len/2, grainsize);
    parallel_sort(data + len/2, len - len/2, grainsize);
    future.wait();
    std::inplace_merge(data, data + len/2, data + len, std::less<T>());
  }
}

}  // namespace synchrolib
