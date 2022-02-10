#pragma once
#include <synchrolib/algorithm/algorithm.hpp>
#include <synchrolib/data_structures/automaton.hpp>
#include <synchrolib/data_structures/pairs_tree.hpp>
#include <synchrolib/data_structures/subset.hpp>
#include <synchrolib/utils/vector.hpp>
#include <synchrolib/utils/general.hpp>
#include <synchrolib/utils/logger.hpp>
#include <synchrolib/utils/timer.hpp>
#include <cassert>
#include <cstring>
#include <optional>
#include <utility>
#include <vector>

#ifndef __INTELLISENSE__
$EPPSTEIN_DEF$
#endif

namespace synchrolib {

template <uint N, uint K>
class Eppstein : public Algorithm<N, K> {
public:
  void run(AlgoData<N, K>& data) override {
    auto log_name = Logger::set_log_name("eppstein");

    if (data.result.non_synchro) {
      return;
    }

    if (data.result.mlsw_lower_bound == data.result.mlsw_upper_bound) {
      return;
    }

    if (data.result.reduce && data.result.reduce->done) {
      Logger::error()
          << "Algorithm is not compatible with inputs after reduction";
      return;
    }

#if FIND_WORD && TRANSITION_TABLES
    Logger::warning() << "Cannot find synchronizing word (not just length) "
                         "when using transition tables";
#endif

#if TRANSITION_TABLES
    PairsTreeTransitionTables<N, K> pairs_tree;
#else
    PairsTree<N, K> pairs_tree;
#endif
    pairs_tree.initialize(data.aut);

    if (!pairs_tree.is_synchronizing()) {
      Logger::warning() << "Automaton is non-synchronizing";
      data.result.non_synchro = true;
      return;
    }

    FastVector<uint> word;
    auto lsw = get_automaton_lsw_eppstein(data, pairs_tree, word);
    if (!lsw) {
      Logger::info()
          << "Failed to find a synchronizing word shorter than the upper bound";
    } else {
      Logger::info() << "Upper bound: " << *lsw;
      assert(*lsw <= data.result.mlsw_upper_bound);
      data.result.mlsw_upper_bound = *lsw;

#if FIND_WORD && !TRANSITION_TABLES
      data.result.word = std::move(word);
#endif
    }

    assert(data.result.mlsw_lower_bound <= data.result.mlsw_upper_bound);
  }

private:
  /* Standard Eppstein algorithm O(n^3+ln), O(n^3)
   */
  template <class PT>
  std::optional<uint64> get_automaton_lsw_eppstein(const AlgoData<N, K>& data,
      const PT& pairs_tree, FastVector<uint>& word) {
    Timer timer_eppstein("get_automaton_lsw_eppstein");
    auto subset = Subset<N>::Complete();
    uint64 len = 0;
    do {
      uint n1 = 0, n2 = 0, l = N * N;
      for (uint v1 = 0; v1 < N - 1; v1++) {
        if (subset.is_set(v1)) {
          for (uint v2 = v1 + 1; v2 < N; v2++) {
            if (subset.is_set(v2)) {
              if (pairs_tree.get_length(v1, v2) < l) {
                n1 = v1;
                n2 = v2;
                l = pairs_tree.get_length(v1, v2);
              }
            }
          }
        }
      }
      len += l;
      if (len > data.result.mlsw_upper_bound) {
        return std::nullopt;
      }
#if FIND_WORD && !TRANSITION_TABLES
      pairs_tree.apply(data.aut, n1, n2, subset, word);
#else
      pairs_tree.apply(data.aut, n1, n2, subset);
#endif
    } while (subset.size() > 1);

    return len;
  }

  /* Cycle algorithm O(n^2+ln), O(min(n^3,ln))
   */
  template <class PT>
  uint64 get_automaton_lsw_cycle(
      const AlgoData<N, K>& data, const PT& pairs_tree) {
    Subset<N> subset = Subset<N>::Complete();
    uint64 len = 0;
    uint n1 = 0, n2 = 0, l = N * N;
    for (uint v1 = 0; v1 < N - 1; v1++) {
      if (subset.is_set(v1)) {
        for (uint v2 = v1 + 1; v2 < N; v2++) {
          if (subset.is_set(v2)) {
            if (pairs_tree.get_length(v1, v2) < l) {
              n1 = v1;
              n2 = v2;
              l = pairs_tree.get_length(v1, v2);
            }
          }
        }
      }
    }
    do {
      len += l;
      if (len >= data.result.mlsw_upper_bound)
        return data.result.mlsw_upper_bound;
      pairs_tree.apply(data.aut, n1, n2, subset);
      l = N * N;
      for (uint v2 = 0; v2 < N; v2++) {
        if (subset.is_set(v2) && v2 != n1) {
          if (pairs_tree.get_length(n1, v2) < l) {
            n2 = v2;
            l = pairs_tree.get_length(n1, v2);
          }
        }
      }
    } while (subset.size() > 1);
    return len;
  }
};

}  // namespace synchrolib

#ifndef __INTELLISENSE__
$EPPSTEIN_UNDEF$
#endif
