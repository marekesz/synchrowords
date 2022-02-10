#pragma once
#include <jitdefines.hpp>

#include <algorithm>
#include <synchrolib/algorithm/algorithm.hpp>
#include <synchrolib/algorithm/exact/utils.hpp>
#include <synchrolib/data_structures/automaton.hpp>
#include <synchrolib/data_structures/preprocessed_transition.hpp>
#include <synchrolib/data_structures/subset.hpp>
#include <synchrolib/data_structures/subsets_trie.hpp>
#include <synchrolib/utils/connectivity.hpp>
#include <synchrolib/utils/vector.hpp>
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
class MeetInTheMiddle : public MemoryUsage {
public:
  MeetInTheMiddle(
      const Automaton<N, K>& aut,
      const InverseAutomaton<N, K>& invaut,
      const std::array<PreprocessedTransition<N, K>, K>& ptrans,
      const std::array<PreprocessedTransition<N, K>, K>& invptrans,
      uint64& reset_threshold,
      FastVector<Subset<N>>& list_bfs,
      FastVector<Subset<N>>& list_invbfs,
      uint64 max_reset_threshold,
      size_t max_memory);

  bool run();

private:
  const Automaton<N, K>& aut;
  const InverseAutomaton<N, K>& invaut;
  const std::array<PreprocessedTransition<N, K>, K>& ptrans;
  const std::array<PreprocessedTransition<N, K>, K>& invptrans;
  uint64& reset_threshold;
  FastVector<Subset<N>>& list_bfs;
  FastVector<Subset<N>>& list_invbfs;

  uint64 max_reset_threshold;
  size_t max_memory;

  FastVector<Subset<N>> list_bfs_visited;
  FastVector<Subset<N>> list_invbfs_visited;

  uint64 last_reduction_bfs_visited_size;
  uint64 last_reduction_invbfs_visited_size;
  uint64 last_bfs_list_size;
  uint64 last_invbfs_list_size;
  uint64 steps_bfs;
  uint64 steps_invbfs;

  struct Decision {
    bool bfs_novisited;     // list_bfs_visited cleared
    bool invbfs_novisited;  // list_invbfs_visited cleared

    enum class Phase {
      BFS, IBFS, IDFS
    };
    Phase phase;

    Decision(): bfs_novisited(false), invbfs_novisited(false) {}
  };

  Decision decision;

  struct ReductionCalculator{
    ReductionCalculator(size_t initial): initial_(initial) {}
    double calculate(size_t current) const {
      if (current > initial_) {
        throw std::runtime_error("reduction made the list bigger");
      }
      return static_cast<double>(initial_ - current) / initial_;
    }
    size_t initial_;
  };

  struct ReductionHistory {
    double reduced_duplicates;
    double reduced_visited;
    double reduced_self;

    ReductionHistory(): reduced_duplicates(0), reduced_visited(0), reduced_self(0) {}
  };

  ReductionHistory bfs_reduction_history;
  ReductionHistory invbfs_reduction_history;
  
  size_t get_memory_usage() const override;

  bool out_of_memory_dfs(size_t list_size) const;

  void process_bfs_step();
  void process_invbfs_step();
  void bfs_step(
      const Automaton<N, K>& aut, const InverseAutomaton<N, K>& invaut);
  void invbfs_step(
      const Automaton<N, K>& aut, const InverseAutomaton<N, K>& invaut);


  void calculate_decision();

  using cost_t = long double;
  static uint64 get_cardinalities(const FastVector<Subset<N>>& list);
  static uint64 get_negated_cardinalities(const FastVector<Subset<N>>& list);
  static double get_trie_evn(const cost_t m, const cost_t p, const cost_t q);
};

}  // namespace synchrolib

#ifndef __INTELLISENSE__
$EXACT_UNDEF$
#endif
