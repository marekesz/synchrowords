#pragma once
#include <jitdefines.hpp>

#include <algorithm>
#include <synchrolib/algorithm/algorithm.hpp>
#include <synchrolib/algorithm/exact/utils.hpp>
#include <synchrolib/algorithm/exact/meet_in_the_middle.hpp>
#include <synchrolib/data_structures/automaton.hpp>
#include <synchrolib/data_structures/preprocessed_transition.hpp>
#include <synchrolib/utils/vector.hpp>
#include <synchrolib/data_structures/subset.hpp>
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
#include <future>
#include <optional>
#include <utility>

#ifndef __INTELLISENSE__
$EXACT_DEF$
#endif

namespace synchrolib {

template <uint N, uint K>
class Dfs : public MemoryUsage {
public:
  Dfs(
      Automaton<N, K>& aut,
      InverseAutomaton<N, K>& invaut,
      std::array<PreprocessedTransition<N, K>, K>& ptrans,
      std::array<PreprocessedTransition<N, K>, K>& invptrans,
      uint64& reset_threshold,
      FastVector<Subset<N>>& list_bfs,
      FastVector<Subset<N>>& list_invbfs,
      uint64 max_depth,
      size_t max_memory);

  bool run();

private:
  using Iterator = typename FastVector<Subset<N>>::iterator;
  using ConstIterator = typename FastVector<Subset<N>>::const_iterator;

  Automaton<N, K>& aut;
  InverseAutomaton<N, K>& invaut;
  std::array<PreprocessedTransition<N, K>, K>& ptrans;
  std::array<PreprocessedTransition<N, K>, K>& invptrans;
  uint64& reset_threshold;
  FastVector<Subset<N>>& list_bfs;
  FastVector<Subset<N>>& list_invbfs;

  uint64 max_depth;
  size_t dfs_max_list_size;
  SubsetsTrie<N, THREADS> trie_bfs;
  size_t max_memory;

  size_t get_memory_usage() const override;

  FastVector<uint> get_order();

  void prepare();

  void process_invdfs(size_t begin, size_t end, const uint64 lsw, const uint64 depth);
  std::tuple<bool, size_t, size_t> invbfs_step_dfs(size_t begin, size_t end, bool reduce_duplicates, size_t reduce_subsets);

  bool check_nextlist_multithreaded(size_t begin, size_t end);

  void update_dfs_max_list_size();
  
  using cost_t = long double;
};

}  // namespace synchrolib

#ifndef __INTELLISENSE__
$EXACT_UNDEF$
#endif
