#pragma once
// #include <external/uwr/vector.hpp>
#include <external/uwr/vector.hpp>
#include <future>
// #include <vector>

namespace synchrolib {

template<typename T>
using FastVector = uwr::vector<T>;

template<typename T>
void keep_unique(FastVector<T>& vec) {
  vec.resize(std::distance(vec.begin(), std::unique(vec.begin(), vec.end())));
}

template<typename T, class Comp=std::less<T>>
void sort_keep_unique(FastVector<T>& vec, Comp comp=Comp{}) {
  std::sort(vec.begin(), vec.end(), comp);
  keep_unique(vec);
}

template<class T, class Comp=std::less<T>>
void parallel_sort(T* data, int len, int grainsize, Comp comp=Comp{}) {
  if (len < grainsize) {
    std::sort(data, data + len, comp);
  } else {
    auto future = std::async(parallel_sort<T, Comp>, data, len/2, grainsize, comp);
    parallel_sort<T, Comp>(data + len/2, len - len/2, grainsize, comp);
    future.wait();
    std::inplace_merge(data, data + len/2, data + len, comp);
  }
}

}  // namespace synchrolib
