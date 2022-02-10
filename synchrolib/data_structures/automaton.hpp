#pragma once
#include <synchrolib/utils/general.hpp>
#include <synchrolib/utils/vector.hpp>
#include <algorithm>
#include <array>
#include <cassert>
#include <sstream>
#include <string>
#include <utility>

#include <synchrolib/data_structures/automaton/automaton.hpp>
#include <synchrolib/data_structures/automaton/inverse_automaton.hpp>
#include <synchrolib/data_structures/automaton/var_automaton.hpp>

namespace synchrolib {

constexpr uint MAX_MLSW(uint n) { return (n - 1) * (n - 1); }

template <uint N, uint K>
inline VarAutomaton get_automaton_reduced(
    const Automaton<N, K>& aut, const FastVector<bool>& mark) {
  uint* translation = new uint[N];
  uint rn = 0;
  for (uint n = 0; n < N; n++)
    if (mark[n]) translation[n] = rn++;
  VarAutomaton raut(rn, K);
  for (uint n = 0; n < N; n++) {
    if (mark[n])
      for (uint k = 0; k < K; k++)
        raut[translation[n]][k] = translation[aut[n][k]];
  }
  delete[] translation;
  return raut;
}

template <typename Container>
static FastVector<uint> get_permutation_from_weights_ascending(
    const Container& weights) {
  FastVector<std::pair<double, uint>> ver(weights.size());
  for (uint i = 0; i < weights.size(); ++i) {
    ver[i] = {weights[i], i};
  }
  std::sort(ver.begin(), ver.end());

  FastVector<uint> order(weights.size());
  for (uint i = 0; i < weights.size(); ++i) {
    order[ver[i].second] = i;
  }
  return order;
}

}  // namespace synchrolib
