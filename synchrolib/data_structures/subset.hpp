#pragma once
#include <synchrolib/data_structures/automaton/automaton.hpp>
#include <synchrolib/data_structures/automaton/inverse_automaton.hpp>
#include <synchrolib/utils/bits.hpp>
#include <synchrolib/utils/general.hpp>
#include <functional>
#include <bitset>

namespace synchrolib {

constexpr uint SUBSETS_BITS = 64;
constexpr uint SUBSET_SIZE_FOR_N(uint n) {
  return (n + SUBSETS_BITS - 1) / SUBSETS_BITS;
}

template <uint S>
struct Subset {
  static constexpr auto SS = SUBSET_SIZE_FOR_N;
  static constexpr uint buckets() { return SS(S); }

  uint64 v[SS(S)];

  static inline Subset<S> Empty() {
    Subset<S> s;
    std::fill(s.v, s.v + SS(S), 0);
    return s;
  }
  static inline Subset<S> Singleton(const uint n) {
    Subset<S> s;
    std::fill(s.v, s.v + SS(S), 0);
    s.v[n / SUBSETS_BITS] = static_cast<uint64>(1) << (n % SUBSETS_BITS);
    return s;
  }

  static inline Subset<S> Complete() {
    Subset<S> s;
    for (uint b = 0; b < S / SUBSETS_BITS; b++)
      s.v[b] = POWERS2M1_64[SUBSETS_BITS];
    if (S / SUBSETS_BITS < SS(S)) {
      s.v[S / SUBSETS_BITS] = POWERS2M1_64[S % SUBSETS_BITS];
      for (uint b = S / SUBSETS_BITS + 1; b < SS(S); b++)
        s.v[b] = 0;  // TODO: why is it needed? (delete)
    }
    return s;
  };

  template <typename Container>
  static inline Subset<S> Permutation(
      const Subset<S> &set, const Container &order) {
    auto ret = Subset<S>::Empty();
    for (uint i = 0; i < S; i++)
      if (set.is_set(i)) ret.set(order[i]);
    return ret;
  }

  __attribute__((pure, hot)) inline bool operator==(const Subset<S> &s) const {
    for (uint b = buckets() - 1; b > 0; b--)
      if (v[b] != s.v[b]) return false;
    return (v[0] == s.v[0]);
  }
  __attribute__((pure, hot)) inline bool operator!=(const Subset<S> &s) const {
    return !((*this) == s);
  }
  __attribute__((pure, hot)) inline bool operator<(const Subset<S> &s) const {
    for (uint b = 0; b < buckets(); ++b) { // TODO: use faster < when order is not important
      if (v[b] == s.v[b]) continue;
      return reverse64(v[b]) < reverse64(s.v[b]);
    }
    return false;
  }
  inline Subset<S>& operator&=(const Subset<S> &s) {
    for (uint b = 0; b < buckets(); b++) v[b] &= s.v[b];
    return *this;
  }
  __attribute__((pure)) inline Subset<S> operator&(const Subset<S> &s) const {
    Subset<S> r = *this;
    r &= s;
    return r;
  }
  inline Subset<S>& operator|=(const Subset<S> &s) {
    for (uint b = 0; b < buckets(); b++) v[b] |= s.v[b];
    return *this;
  }
  __attribute__((pure)) inline Subset<S> operator|(const Subset<S> &s) const {
    Subset<S> r = *this;
    r |= s;
    return r;
  }

  inline void negate() {
    for (uint b = 0; b < buckets(); ++b) {
      v[b] = ~v[b];
    }
#if GPU
    if (S % SUBSETS_BITS != 0) { // TODO: change nvcc version to support if constexpr
#else
    if constexpr (S % SUBSETS_BITS != 0) {
#endif
      v[buckets() - 1] &= POWERS2_64[S % SUBSETS_BITS] - 1;
    }
  }

  inline Subset<S> operator~() const {
    auto sub = *this;
    sub.negate();
    return sub;
  }

  template <typename T>
  __attribute__((hot)) inline void count(T* acc) {
    for (uint b = 0; b < buckets(); b++) {
      uint n = b * SUBSETS_BITS;
      uint64 c = v[b];
      while (c) {
        int shift = __builtin_ctzll(c);
        c >>= shift;
        n += shift;
        ++acc[n];
        c ^= 1;
      }
    }
  };

  template <uint N, uint K>
  __attribute__((hot)) inline void apply(
      const Automaton<N, K> &aut, const uint k, Subset<S> &s) const {
    for (uint b = 0; b < buckets(); b++) s.v[b] = 0;
    for (uint b = 0; b < buckets(); b++) {
      uint n = b * SUBSETS_BITS;
      uint64 c = v[b];
      while (c) {
        int shift = __builtin_ctzll(c);
        c >>= shift;
        n += shift;
        s.set(aut[n][k]);
        c ^= 1;
      }
      // while (true) {
      //   if (c & 1)
      //     s.set(aut[n][k]);
      //   else if (!c)
      //     break;
      //   c >>= 1;
      //   n++;
      // }
    }
  }

  // template<int N,int K> __attribute__((hot)) inline void apply(const
  // Automaton<N,K> &aut,const DynamicWord &word,Subset<N> &s) const { Subset<N>
  // subset; 	s = *this; 	for (uint i=0;i<word.size();i++) {
  // 		s.apply(aut,word[i],subset);
  // 		s = subset;
  // 	}
  // }

  template <uint N, uint K>
  __attribute__((hot)) inline void invapply(
      const InverseAutomaton<N, K> &invaut, const uint k, Subset<S> &s) const {
    for (uint b = 0; b < buckets(); b++) s.v[b] = 0;
    for (uint b = 0; b < buckets(); b++) {
      uint n = b * SUBSETS_BITS;
      uint64 c = v[b];
      while (c) {
        int shift = __builtin_ctzll(c);
        c >>= shift;
        n += shift;
        for (uint e = invaut.begin(n, k); e < invaut.end(n, k); e++) {
          s.set(invaut.edge(e, k));
        }
        c ^= 1;
      }
    }
  }

  __attribute__((pure, hot)) inline bool is_set(const uint n) const {
    return (
        v[n / SUBSETS_BITS] & ((unsigned long long)1 << (n % SUBSETS_BITS)));
  }
  __attribute__((hot)) inline void set(const uint n) {
    v[n / SUBSETS_BITS] |= ((unsigned long long)1 << (n % SUBSETS_BITS));
  }
  inline void unset(const uint n) {
    v[n / SUBSETS_BITS] =
        ~(~v[n / SUBSETS_BITS] | ((unsigned long long)1 << (n % SUBSETS_BITS)));
  }
  __attribute__((pure, hot)) inline uint size() const {
    uint s = count_bits64(v[0]);
    for (uint b = 1; b < buckets(); b++) s += count_bits64(v[b]);
    return s;
  }
  __attribute__((pure)) __attribute__((hot)) inline bool is_subset(
      const Subset<S> &subset) const {
    for (uint b = 0; b < buckets(); b++)
      if ((subset.v[b] & v[b]) != subset.v[b]) return false;
    return true;
  }
  __attribute__((pure)) __attribute__((hot)) inline bool is_proper_subset(
      const Subset<S> &subset) const {
    return *this != subset && is_subset(subset);
  }
};

}  // namespace synchrolib
