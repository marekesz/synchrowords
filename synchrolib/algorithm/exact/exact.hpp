#pragma once
#include <jitdefines.hpp>

#include <algorithm>
#include <synchrolib/algorithm/algorithm.hpp>
#include <synchrolib/algorithm/exact/utils.hpp>
#include <synchrolib/algorithm/exact/meet_in_the_middle.hpp>
#include <synchrolib/algorithm/exact/dfs.hpp>
#include <synchrolib/data_structures/automaton.hpp>
#include <synchrolib/data_structures/preprocessed_transition.hpp>
#include <synchrolib/data_structures/subset.hpp>
#include <synchrolib/utils/vector.hpp>
#include <synchrolib/data_structures/subsets_trie.hpp>
#include <synchrolib/data_structures/subsets_implicit_trie.hpp>
#include <synchrolib/utils/connectivity.hpp>
#include <synchrolib/utils/distribution.hpp>
#include <synchrolib/utils/general.hpp>
#include <synchrolib/utils/logger.hpp>
#include <cassert>
#include <cmath>
#include <cstring>
#include <exception>
#include <map>
#include <numeric>
#include <optional>
#include <utility>

#ifndef __INTELLISENSE__
$EXACT_DEF$
#endif

namespace synchrolib {

template <uint N, uint K>
class Exact : public Algorithm<N, K>, public MemoryUsage {
public:
  void run(AlgoData<N, K>& data) override;

private:
  Automaton<N, K> aut;
  InverseAutomaton<N, K> invaut;

  uint64 reset_threshold;
  size_t max_memory;

  std::array<PreprocessedTransition<N, K>, K> ptrans;
  std::array<PreprocessedTransition<N, K>, K> invptrans;

  FastVector<Subset<N>> list_bfs;
  FastVector<Subset<N>> list_invbfs;

  static constexpr size_t MEMORY_RESERVE =
      1024 * 1024 * 16;                        // 16mb reserved for misc objects

  size_t get_memory_usage() const override;

  static FastVector<uint> get_automaton_order(const Automaton<N, K>& aut, const InverseAutomaton<N, K>& invaut);
  void initialize_bfs_lists(const AlgoData<N, K>& data);
  void initialize_invbfs_lists(const AlgoData<N, K>& data);
  void calculate_max_memory();
  void set_automaton_and_reorder(const AlgoData<N, K>& data);
  void preprocess_transitions();

  bool run_meet_in_the_middle(uint64 max_reset_threshold);
  bool run_dfs(uint64 max_reset_threshold);
};

}  // namespace synchrolib

#ifndef __INTELLISENSE__
$EXACT_UNDEF$
#endif
