#pragma once
#include <algorithm>
#include <array>
#include <synchrolib/utils/general.hpp>
#include <cassert>
#include <sstream>
#include <string>
#include <utility>

namespace synchrolib {

template <uint N, uint K>
class InverseAutomaton {
private:
  uint edges_[N * K];
  uint begin_[N * K];
  uint end_[N * K];

public:
  constexpr InverseAutomaton() : edges_{}, begin_{}, end_{} {}

  constexpr InverseAutomaton(std::array<uint, 3 * N * K> array)
      : edges_{}, begin_{}, end_{} {
    for (uint i = 0; i < N * K; ++i) {
      edges_[i] = array[i];
    }
    for (uint i = 0; i < N * K; ++i) {
      begin_[i] = array[N * K + i];
    }
    for (uint i = 0; i < N * K; ++i) {
      end_[i] = array[2 * N * K + i];
    }
  }

  InverseAutomaton(const Automaton<N, K>& aut) {
    for (uint i = 0; i < N * K; ++i) {
      end_[i] = 0;
    }
    for (uint k = 0; k < K; ++k) {
      for (uint n = 0; n < N; ++n) {
        end_[k * N + aut[n][k]]++;
      }
    }

    for (uint k = 0; k < K; ++k) {
      begin_[k * N] = 0;
      for (uint n = 1; n < N; ++n) {
        begin_[k * N + n] = begin_[k * N + n - 1] + end_[k * N + n - 1];
      }
      for (uint n = 0; n < N; ++n) {
        end_[k * N + n] = begin_[k * N + n];  // fno-tree-vectorize
      }
    }

    for (uint k = 0; k < K; ++k) {
      for (uint n = 0; n < N; ++n) {
        edges_[k * N + end_[k * N + aut[n][k]]++] = n;
      }
    }
  }

  __attribute__((always_inline)) constexpr inline uint edge(
      const uint e, const uint k) const {
    return edges_[k * N + e];
  }
  __attribute__((always_inline)) constexpr inline uint begin(
      const uint n, const uint k) const {
    return begin_[k * N + n];
  }
  __attribute__((always_inline)) constexpr inline uint end(
      const uint n, const uint k) const {
    return end_[k * N + n];
  }

  std::string code() const {
    std::stringstream ss;
    for (uint i = 0; i < N * K; ++i) {
      if (i != 0) ss << ", ";
      ss << edges_[i];
    }
    for (uint i = 0; i < N * K; ++i) {
      ss << ", " << begin_[i];
    }
    for (uint i = 0; i < N * K; ++i) {
      ss << ", " << end_[i];
    }
    return ss.str();
  }
};

} // namespace synchrolib