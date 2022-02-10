#pragma once
#include <synchrolib/data_structures/automaton.hpp>
#include <synchrolib/data_structures/subset.hpp>
#include <synchrolib/utils/general.hpp>
#include <synchrolib/utils/vector.hpp>
#include <functional>
#include <bitset>

namespace synchrolib {

template <uint N, bool Ascending=false>
void sort_sets_cardinality_descending(
    typename FastVector<Subset<N>>::iterator begin,
    typename FastVector<Subset<N>>::iterator end,
    std::function<void(
      typename FastVector<Subset<N>>::iterator,
      typename FastVector<Subset<N>>::iterator)> fun) {
  size_t size = std::distance(begin, end);

  std::array<uint, N + 1> counts{};
  for (auto it = begin; it != end; ++it) {
    if constexpr (Ascending) {
      counts[N - it->size()]++;
    } else {
      counts[it->size()]++;
    }
  }

  uint sum = 0;
  for (int i = N; i >= 0; i--) {
    uint c = counts[i];
    counts[i] = sum;
    sum += c;
  }
  auto start_pos = counts;

  FastVector<Subset<N>> tmp(size);
  for (auto it = begin; it != end; ++it) {
    if constexpr (Ascending) {
      tmp[counts[N - it->size()]++] = *it;
    } else {
      tmp[counts[it->size()]++] = *it;
    }
  }
  std::copy(tmp.begin(), tmp.end(), begin);

  for (uint i = 0; i <= N; ++i) {
    fun(begin + start_pos[i], begin + counts[i]);
  }
}

template <uint N, bool Ascending=false>
void sort_sets_cardinality_descending(typename FastVector<Subset<N>>::iterator begin,
    typename FastVector<Subset<N>>::iterator end) {
  sort_sets_cardinality_descending<N, Ascending>(begin, end, [](auto a, auto b){});
}

template <uint S>
__attribute__((pure, hot)) bool subsets_size_invlex_comparator(
    const Subset<S> &s1, const Subset<S> &s2) {
  uint size1 = s1.size(), size2 = s2.size();
  if (size1 < size2) return true;
  if (size1 > size2) return false;
  for (uint b = 0; b < s1.buckets(); b++) {
    uint64 v1 = reverse64(s1.v[b]);
    uint64 v2 = reverse64(s2.v[b]);
    if (v1 > v2) return true;
    if (v1 < v2) return false;
  }
  return false;
}

template <uint S>
__attribute__((pure, hot)) bool subsets_invsize_backinvlex_comparator(
    const Subset<S> &s1, const Subset<S> &s2) {
  uint size1 = s1.size(), size2 = s2.size();
  if (size2 < size1) return true;
  if (size2 > size1) return false;
  for (uint b = s1.buckets() - 1; b > 0; b--) {
    if (s1.v[b] > s2.v[b]) return true;
    if (s1.v[b] < s2.v[b]) return false;
  }
  return s1.v[0] > s2.v[0];
}

template <uint N>
std::ostream &operator<<(std::ostream &stream, const Subset<N> &set) {
  for (uint i = 0; i < SUBSET_SIZE_FOR_N(N); ++i) {
    auto str = std::bitset<SUBSETS_BITS>(set.v[i]).to_string();
    std::reverse(str.begin(), str.end());
    stream << str;
  }
  return stream;
}

}  // namespace synchrolib
