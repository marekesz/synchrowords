#pragma once
#include <jitdefines.hpp>

#include <synchrolib/data_structures/automaton/automaton.hpp>
#include <synchrolib/data_structures/automaton/inverse_automaton.hpp>
#include <synchrolib/data_structures/subset.hpp>
#include <synchrolib/utils/bits.hpp>
#include <synchrolib/utils/general.hpp>

namespace synchrolib {

// Must be a power of 2
constexpr uint PREPROCESSED_TRANSITION_SLICE = 8;
constexpr uint PREPROCESSED_TRANSITION_NUM_SLICES_FOR_N(uint n) {
  return (
      (n + PREPROCESSED_TRANSITION_SLICE - 1) / PREPROCESSED_TRANSITION_SLICE);
}

template <uint N, uint K>
struct PreprocessedTransition {
  static constexpr auto NS = PREPROCESSED_TRANSITION_NUM_SLICES_FOR_N;
  static constexpr uint slices() { return NS(N); }

  Subset<N> trans[NS(N)][(1 << PREPROCESSED_TRANSITION_SLICE)];

  PreprocessedTransition();
  PreprocessedTransition(const Automaton<N, K> &aut, const uint k);
  PreprocessedTransition(const InverseAutomaton<N, K> &invaut, const uint k);

  void apply(const Subset<N>* from, Subset<N>* to, size_t cnt) const;

  __attribute__((hot)) inline void apply(const Subset<N> &from, Subset<N> &s) const {
    for (uint b = 0; b < s.buckets(); b++) s.v[b] = 0;
    for (uint i = 0; i < slices(); i++) {
      const uint b = i * PREPROCESSED_TRANSITION_SLICE / SUBSETS_BITS;
      const uint shift = i * PREPROCESSED_TRANSITION_SLICE % SUBSETS_BITS;
      const uint set = (uint)(
          (from.v[b] >> (shift)) & ((1 << PREPROCESSED_TRANSITION_SLICE) - 1));
      s |= trans[i][set];
    }
  }
};

}  // namespace synchrolib