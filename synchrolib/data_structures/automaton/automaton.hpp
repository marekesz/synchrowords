#pragma once
#include <algorithm>
#include <array>
#include <synchrolib/utils/general.hpp>
#include <cassert>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace synchrolib {

template <uint N, uint K>
class Automaton {
private:
  uint t[N * K];

public:
  constexpr Automaton() : t{} {}

  constexpr Automaton(std::array<uint, N * K> array) : t{} {
    for (uint i = 0; i < N * K; ++i) {
      t[i] = array[i];
    }
  }

  __attribute__((always_inline)) constexpr inline uint* operator[](uint pos) {
    return t + (K * pos);
  }
  __attribute__((always_inline)) constexpr inline const uint* operator[](
      uint pos) const {
    return t + (K * pos);
  }

  std::string encode() const {
    std::stringstream ss;
    for (uint i = 0; i < N * K; ++i) {
      if (i != 0) ss << " ";
      ss << t[i];
    }
    return ss.str();
  }

  static Automaton<N, K> decode(const std::string& s) {
    Automaton<N, K> aut;
    std::stringstream ss(s);
    for (uint i = 0; i < N * K; ++i) {
      ss >> aut.t[i];
      assert(aut.t[i] < N);
    }
    return aut;
  }

  template <typename Container>
  static Automaton<N, K> permutation(
      const Automaton<N, K>& aut, const Container& order) {
    Automaton<N, K> ret;
    for (uint n = 0; n < N; ++n) {
      for (uint k = 0; k < K; ++k) {
        ret[order[n]][k] = order[aut[n][k]];
      }
    }
    return ret;
  }

  template <typename Container>
  static Automaton<N, K> reordered_ascending(
      const Automaton<N, K>& aut, const Container& weights) {
    auto order = get_permutation_from_weights_ascending(aut, weights);
    return Automaton<N, K>::permutation(aut, order);
  }

  static Automaton<N, K> generate_random(int seed) {
    srand48(seed);
    Automaton<N, K> aut;
    for (uint n = 0; n < N; ++n) {
      for (uint k = 0; k < K; ++k) {
        aut[n][k] = static_cast<uint>(drand48() * N);
      }
    }
    return aut;
  }

  static Automaton<N, K> generate_cerny(uint special_pos) {
    static_assert(K == 2);
    static_assert(N > 1);
    Automaton<N, K> aut;
    for (uint n = 0; n < N; ++n) {
      aut[n][0] = n + 1;
      aut[n][1] = n;
    }
    aut[N - 1][0] = 0;
    aut[special_pos][1] = 1;
    return aut;
  }

  // 10 11 12 13 14 15 16 17 18 19 20
  // 29 35 40 44 49 54 59 63 69 76 82
  // mlsw=141 @ n=30
  // mlsw=173 @ n=35
  // mlsw=193 @ n=38
  static Automaton<N, K> generate_hard() {
    static_assert(K == 2);
    static_assert(N > 3);
    Automaton<N, K> aut;
    for (uint i = 0; i < N-1; ++i) {
      aut[i][0] = aut[i][1] = i+1;
    }
    aut[N-1][0] = 0;
    aut[N-1][1] = 1;
    aut[1][1] = 0;
    aut[0][1] = 3;
    aut[2][1] = 3;
    return aut;
  }
};

} // namespace synchrolib