#include <jitdefines.hpp>
#ifdef COMPILE_EXACT

#include "meet_in_the_middle.hpp"

#include <algorithm>
#include <synchrolib/algorithm/algorithm.hpp>
#include <synchrolib/algorithm/exact/utils.hpp>
#include <synchrolib/data_structures/automaton.hpp>
#include <synchrolib/data_structures/preprocessed_transition.hpp>
#include <synchrolib/data_structures/subset.hpp>
#include <synchrolib/data_structures/subsets_trie.hpp>
#include <synchrolib/utils/vector.hpp>
#include <synchrolib/data_structures/subsets_implicit_trie.hpp>
#include <synchrolib/utils/connectivity.hpp>
#include <synchrolib/utils/distribution.hpp>
#include <synchrolib/utils/general.hpp>
#include <synchrolib/utils/logger.hpp>
#include <synchrolib/utils/timer.hpp>
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

template<uint N, uint K>
MeetInTheMiddle<N, K>::MeetInTheMiddle(
    const Automaton<N, K>& aut,
    const InverseAutomaton<N, K>& invaut,
    const std::array<PreprocessedTransition<N, K>, K>& ptrans,
    const std::array<PreprocessedTransition<N, K>, K>& invptrans,
    uint64& reset_threshold,
    FastVector<Subset<N>>& list_bfs,
    FastVector<Subset<N>>& list_invbfs,
    uint64 max_reset_threshold,
    size_t max_memory):
  aut(aut),
  invaut(invaut),
  ptrans(ptrans),
  invptrans(invptrans),
  reset_threshold(reset_threshold),
  list_bfs(list_bfs),
  list_invbfs(list_invbfs),
  max_reset_threshold(max_reset_threshold),
  max_memory(max_memory) {
}

template<uint N, uint K>
bool MeetInTheMiddle<N, K>::run() {
  Timer timer("meet_in_the_middle");

  last_reduction_bfs_visited_size = last_reduction_invbfs_visited_size = 0;
  last_bfs_list_size = last_invbfs_list_size = 0;
  steps_bfs = steps_invbfs = 0;

  // TODO: test
  // list_bfs_visited = list_bfs;
  // list_invbfs_visited = list_invbfs;
  // for (auto& sub : list_invbfs_visited) {
  //   sub.negate();
  // }

  bool found = false;
  while (reset_threshold < max_reset_threshold) {
    if (get_memory_usage() > max_memory) {
      Logger::warning() << "Ended by exceeding the memory limit";
      break;
    }

    calculate_decision();
    if (decision.phase == Decision::Phase::IDFS) {
      break;
    }

    reset_threshold++;
    try {
      if (decision.phase == Decision::Phase::BFS) process_bfs_step();
      else process_invbfs_step();

      Timer goal_check("goal_check");
      auto ret = SubsetsImplicitTrie<N, false, THREADS>::check_contains_subset(list_bfs, list_invbfs);
      goal_check.stop();

      if (std::any_of(ret.begin(), ret.end(), [](bool x) { return x; })) {
        Logger::verbose() << "Synchronizing word found at depth " << reset_threshold;
        found = true;
        break;
      }
    } catch (OutOfMemoryException& ex) {
      reset_threshold--;
      Logger::warning() << "Ended by exceeding the memory limit while processing bfs/ibfs step";
      break;
    }
  }

  list_bfs_visited = list_invbfs_visited = FastVector<Subset<N>>();
  return found;
}

template<uint N, uint K>
size_t MeetInTheMiddle<N, K>::get_memory_usage() const {
  return synchrolib::get_memory_usage(list_bfs) +
      synchrolib::get_memory_usage(list_invbfs) +
      synchrolib::get_memory_usage(list_bfs_visited) +
      synchrolib::get_memory_usage(list_invbfs_visited) +
      synchrolib::get_memory_usage(ptrans) +
      synchrolib::get_memory_usage(invptrans);
}

template<uint N, uint K>
void MeetInTheMiddle<N, K>::process_bfs_step() {
  Logger::verbose() << "(BFS)"
                    << (decision.bfs_novisited ? " [novisited]" : "")
                    << " len: " << reset_threshold
                    << " list size: " << list_bfs.size()
                    << " inv list size: " << list_invbfs.size()
                    << " memory usage: " << get_megabytes(get_memory_usage());
  steps_bfs++;
  bool reduce_visited =
      !decision.bfs_novisited &&
      (list_bfs_visited.size() >= K * K * last_reduction_bfs_visited_size);

  if (reduce_visited) {
    Logger::verbose() << "(BFS) reducing visited list";

    {
      uint max_size_visited = 0;
      for (const auto& x : list_bfs_visited) { if (x.size() > max_size_visited) max_size_visited = x.size(); }

      uint max_size = 0;
      for (const auto& x : list_bfs) { if (x.size() > max_size) max_size = x.size(); }
      auto visited_end = std::remove_if(list_bfs_visited.begin(), list_bfs_visited.end(), [&](const Subset<N> &s){return s.size() > max_size;});
      Logger::debug() << "max_size_visited " << max_size_visited << " max_size " << max_size << " removed " << (list_bfs_visited.end() - visited_end);
    
      list_bfs_visited.erase(visited_end, list_bfs_visited.end());
    }

    SubsetsImplicitTrie<N, true, THREADS>::reduce(list_bfs_visited); // memory checked in calculate_decision()
    last_reduction_bfs_visited_size = list_bfs_visited.size();
  }

  last_bfs_list_size = list_bfs.size();
  bfs_step(aut, invaut);
}

template<uint N, uint K>
void MeetInTheMiddle<N, K>::process_invbfs_step() {
  Logger::verbose() << "(IBFS)"
                    << (decision.invbfs_novisited ? " [novisited]" : "")
                    << " len: " << reset_threshold
                    << " list size: " << list_bfs.size()
                    << " inv list size: " << list_invbfs.size()
                    << " memory usage: " << get_megabytes(get_memory_usage());
  steps_invbfs++;
  bool reduce_visited =
      !decision.invbfs_novisited &&
      (list_invbfs_visited.size() >= K * K * last_reduction_invbfs_visited_size);

  if (reduce_visited) {
    Logger::verbose() << "(IBFS) reducing visited list";
    SubsetsImplicitTrie<N, true, THREADS>::reduce(list_invbfs_visited); // memory checked in calculate_decision()
    last_reduction_invbfs_visited_size = list_invbfs_visited.size();
  }

  last_invbfs_list_size = list_invbfs.size();
  invbfs_step(aut, invaut);
}

template<uint N, uint K>
void MeetInTheMiddle<N, K>::bfs_step(
    const Automaton<N, K>& aut, const InverseAutomaton<N, K>& invaut) {
  const size_t next_size = list_bfs.size() * K;
  if (get_memory_usage() + sizeof(Subset<N>) * next_size > max_memory || out_of_memory_dfs(list_bfs.size())) {
    throw OutOfMemoryException();
  }

  FastVector<Subset<N>> list_next(next_size);
  Timer apply_timer("apply");
  for (uint k = 0; k < K; ++k) {
    ptrans[k].apply(list_bfs.data(), list_next.data() + (k * list_bfs.size()), list_bfs.size());
  }
  apply_timer.stop();

  list_bfs = std::move(list_next);

  if (decision.bfs_novisited) {
    ReductionCalculator reduced_duplicates(list_bfs.size());
    sort_keep_unique(list_bfs);
    bfs_reduction_history.reduced_duplicates = reduced_duplicates.calculate(list_bfs.size());

    if (get_memory_usage() + synchrolib::get_memory_usage(list_bfs) > max_memory) {
      throw OutOfMemoryException();
    }
    ReductionCalculator reduced_visited(list_bfs.size()); // TODO: it should be reduced_self, but it needs to
                                                          //       be compatible with the equations in calculate_decision()
    SubsetsImplicitTrie<N, true, THREADS, true>::reduce(list_bfs);
    bfs_reduction_history.reduced_visited = reduced_visited.calculate(list_bfs.size());
  } else {
    if (get_memory_usage() + synchrolib::get_memory_usage(list_bfs) > max_memory) { // place for visited
      throw OutOfMemoryException();
    }

    sort_keep_unique(list_bfs_visited); // TODO: maybe we don't need to sort here

    ReductionCalculator reduced_duplicates(list_bfs.size());
    sort_keep_unique(list_bfs);
    bfs_reduction_history.reduced_duplicates = reduced_duplicates.calculate(list_bfs.size());

    list_bfs_visited.reserve(list_bfs_visited.size() + list_bfs.size()); // important!
                                                                         // (we don't want reallocation
                                                                         // in the following loop)
    auto it = list_bfs_visited.begin();
    auto end = list_bfs_visited.end();
    for (const auto& sub : list_bfs) {
      while (it != end && *it < sub) {
        ++it;
      }
      if (it == end || *it != sub) {
        list_bfs_visited.push_back(sub); // no realloc
      }
    }
    list_bfs = FastVector<Subset<N>>(end, list_bfs_visited.end());
    std::sort(list_bfs_visited.begin(), list_bfs_visited.end()); // must be sorted and not contain duplicates

    ReductionCalculator reduced_visited(list_bfs.size());
    SubsetsImplicitTrie<N, true, THREADS, true>::reduce(list_bfs_visited, list_bfs);
    bfs_reduction_history.reduced_visited = reduced_visited.calculate(list_bfs.size());
  }

  Logger::debug() << "BFS reduction |"
    << " duplicates: " << bfs_reduction_history.reduced_duplicates
    << " visited + self: " << bfs_reduction_history.reduced_visited;
}

template<uint N, uint K>
void MeetInTheMiddle<N, K>::invbfs_step(
    const Automaton<N, K>& aut, const InverseAutomaton<N, K>& invaut) {
  const size_t next_size = list_invbfs.size() * K;
  if (get_memory_usage() + sizeof(Subset<N>) * next_size > max_memory || out_of_memory_dfs(list_invbfs.size())) {
    throw OutOfMemoryException();
  }

  FastVector<Subset<N>> list_next(next_size);
  for (uint k = 0; k < K; ++k) {
    invptrans[k].apply(list_invbfs.data(), list_next.data() + (k * list_invbfs.size()), list_invbfs.size());
  }
  // for (uint i = 0; i < list_invbfs.size(); i++) {
  //   for (uint k = 0; k < K; k++) {
  //     invptrans[k].apply(list_invbfs[i], list_next[i * K + k]);
  //     // list_invbfs[i].template invapply<N, K>(invaut, k, list_next[i * K + k]);
  //   }
  // }

  list_invbfs = std::move(list_next);

  if (decision.invbfs_novisited) {
    invbfs_reduction_history.reduced_visited = 0;

    for (auto& sub : list_invbfs) {
      sub.negate();
    }

    ReductionCalculator reduced_duplicates(list_invbfs.size());
    sort_keep_unique(list_invbfs);
    invbfs_reduction_history.reduced_duplicates = reduced_duplicates.calculate(list_invbfs.size());

    ReductionCalculator reduced_self(list_invbfs.size());
    if (get_memory_usage() + synchrolib::get_memory_usage(list_invbfs) > max_memory) {
      for (auto& sub : list_invbfs) {
        sub.negate();
      }
      throw OutOfMemoryException();
    }
    SubsetsImplicitTrie<N, true, THREADS, true>::reduce(list_invbfs);
    invbfs_reduction_history.reduced_self = reduced_self.calculate(list_invbfs.size());

    for (auto& sub : list_invbfs) {
      sub.negate();
    }
  } else {
    if (get_memory_usage() + sizeof(Subset<N>) * list_invbfs.size() > max_memory) {
      throw OutOfMemoryException();
    }

    for (auto& sub : list_invbfs) {
      sub.negate();
    }

    sort_keep_unique(list_invbfs_visited);

    ReductionCalculator reduced_duplicates(list_invbfs.size());
    sort_keep_unique(list_invbfs);
    invbfs_reduction_history.reduced_duplicates = reduced_duplicates.calculate(list_invbfs.size());

    ReductionCalculator reduced_self(list_invbfs.size()); // added
    if (get_memory_usage() + synchrolib::get_memory_usage(list_invbfs) > max_memory) {
      for (auto& sub : list_invbfs) {
        sub.negate();
      }
      throw OutOfMemoryException();
    }
    SubsetsImplicitTrie<N, true, THREADS, true>::reduce(list_invbfs);
    invbfs_reduction_history.reduced_self = reduced_self.calculate(list_invbfs.size());
    std::sort(list_invbfs.begin(), list_invbfs.end());


    ReductionCalculator reduced_visited(list_invbfs.size());
    SubsetsImplicitTrie<N, false, THREADS, true>::reduce(list_invbfs_visited, list_invbfs);
    invbfs_reduction_history.reduced_visited = reduced_visited.calculate(list_invbfs.size());

    std::sort(list_invbfs.begin(), list_invbfs.end());

    // ReductionCalculator reduced_self(list_invbfs.size());
    // if (get_memory_usage() + synchrolib::get_memory_usage(list_invbfs) > max_memory) {
    //   for (auto& sub : list_invbfs) {
    //     sub.negate();
    //   }
    //   throw OutOfMemoryException();
    // }
    // SubsetsImplicitTrie<N, true, THREADS, true>::reduce(list_invbfs);
    // invbfs_reduction_history.reduced_self = reduced_self.calculate(list_invbfs.size());

    list_invbfs_visited.reserve(list_invbfs_visited.size() + list_invbfs.size());
    for (const auto& sub : list_invbfs) list_invbfs_visited.push_back(sub);

    for (auto& sub : list_invbfs) {
      sub.negate();
    }
  }

  Logger::debug() << "IBFS reduction |"
    << " duplicates: " << invbfs_reduction_history.reduced_duplicates
    << " visited: " << invbfs_reduction_history.reduced_visited
    << " self: " << invbfs_reduction_history.reduced_self;
}

template<uint N, uint K>
void MeetInTheMiddle<N, K>::calculate_decision() {
  constexpr cost_t DFS_COST_WEIGHT = 0.25;

  // Trivial decision if small lists
  if (list_bfs.size() <= BFS_SMALL_LIST_SIZE || list_invbfs.size() <= BFS_SMALL_LIST_SIZE || !steps_bfs || !steps_invbfs) {
    decision.phase = (list_bfs.size() <= list_invbfs.size() ? Decision::Phase::BFS : Decision::Phase::IBFS);
    return;
  }

  const uint64 card_list_bfs = get_cardinalities(list_bfs);
  const uint64 card_list_invbfs = get_cardinalities(list_invbfs);
  const uint64 card_trie_visited_bfs = get_cardinalities(list_bfs_visited);
  const uint64 card_trie_visited_invbfs = get_negated_cardinalities(list_invbfs_visited);
  const cost_t density_list_bfs = static_cast<cost_t>(card_list_bfs) / (N * list_bfs.size());
  const cost_t density_list_invbfs = static_cast<cost_t>(card_list_invbfs) / (N * list_invbfs.size());

  cost_t cost_bfs, cost_invbfs, cost_shortcut, cost_noshortcut;

  /*
  // Formulas without reduction history
  // Separate reductions:
  //cost_bfs = K * list_bfs.size() * get_trie_evn(list_bfs_visited.size(), density_list_bfs, static_cast<cost_t>(card_trie_visited_bfs) / (N * list_bfs_visited.size()));
  //cost_bfs += K * list_bfs.size() * get_trie_evn(K * list_bfs.size(), density_list_bfs, density_list_bfs);
  //cost_bfs += list_invbfs.size() * get_trie_evn(K * list_bfs.size(), density_list_invbfs, density_list_bfs);
  // Joint reductions:
  cost_bfs = K * list_bfs.size() * get_trie_evn(K * list_bfs.size() + list_bfs_visited.size(), density_list_bfs,
      static_cast<cost_t>(K * card_list_bfs + card_trie_visited_bfs) / (N * (K * list_bfs.size() + list_bfs_visited.size())));
  cost_bfs += list_invbfs.size() * get_trie_evn(K * list_bfs.size(), density_list_invbfs, density_list_bfs);
  
  // Separate reductions:
  cost_invbfs = K * list_invbfs.size() * get_trie_evn(list_invbfs_visited.size(), 1.0 - density_list_invbfs, 1.0 - static_cast<cost_t>(card_trie_visited_invbfs) / (N * list_invbfs_visited.size()));
  cost_invbfs += K * list_invbfs.size() * get_trie_evn(K * list_invbfs.size(), 1.0 - density_list_invbfs, 1.0 - density_list_invbfs);
  cost_invbfs += K * list_invbfs.size() * get_trie_evn(list_bfs.size(), density_list_invbfs, density_list_bfs);
  // Joint reductions:
  //cost_t cost_invbfs = K * list_invbfs.size() * get_trie_evn(K * list_invbfs.size() + list_invbfs_visited.size(), 1.0 - density_list_invbfs,
  //    1.0 - static_cast<cost_t>(K * card_list_invbfs + card_trie_visited_invbfs) / (N * (K * list_invbfs.size() + list_invbfs_visited.size())));
  //cost_t cost_invbfs += K * list_invbfs.size() * get_trie_evn(list_bfs.size(), density_list_invbfs, density_list_bfs);

  if (reset_threshold >= MIN_LENGTH_FOR_SHORTCUT && last_bfs_list_size < list_bfs.size() && last_invbfs_list_size < list_invbfs.size()) {
    cost_shortcut =
        DFS_COST_WEIGHT * list_invbfs.size() * std::pow(K, max_reset_threshold - reset_threshold) *
        get_trie_evn(list_bfs.size(), density_list_invbfs, density_list_bfs);
    cost_noshortcut = cost_bfs +
        DFS_COST_WEIGHT * list_invbfs.size() * std::pow(K, max_reset_threshold - reset_threshold - 1) *
        get_trie_evn(K * list_bfs.size(), density_list_invbfs, density_list_bfs);
    
    if (cost_shortcut < cost_noshortcut) return Decision::IDFS;
  }
  */

  // Step costs

  cost_t branching_bfs_visited = K * (1.0 - bfs_reduction_history.reduced_duplicates);
  cost_t cost_bfs_visited = branching_bfs_visited * list_bfs.size() * get_trie_evn(branching_bfs_visited * list_bfs.size() + list_bfs_visited.size(), density_list_bfs,
      static_cast<cost_t>(branching_bfs_visited * card_list_bfs + card_trie_visited_bfs) / (N * (branching_bfs_visited * list_bfs.size() + list_bfs_visited.size())));
  branching_bfs_visited *= (1.0-bfs_reduction_history.reduced_visited);
  cost_bfs_visited += list_invbfs.size() * get_trie_evn(branching_bfs_visited * list_bfs.size(), density_list_invbfs, density_list_bfs);

  cost_t branching_invbfs_visited = K * (1.0 - invbfs_reduction_history.reduced_duplicates);
  cost_t cost_invbfs_visited = branching_invbfs_visited * list_invbfs.size() * get_trie_evn(branching_invbfs_visited * list_invbfs.size(), 1.0 - density_list_invbfs, 1.0 - density_list_invbfs);
  branching_invbfs_visited *= (1.0 - invbfs_reduction_history.reduced_self);
  cost_invbfs_visited += branching_invbfs_visited * list_invbfs.size() * get_trie_evn(list_invbfs_visited.size(), 1.0 - density_list_invbfs, 1.0 - static_cast<cost_t>(card_trie_visited_invbfs) / (N * list_invbfs_visited.size()));
  branching_invbfs_visited *= (1.0 - invbfs_reduction_history.reduced_visited);
  cost_invbfs_visited += branching_invbfs_visited * list_invbfs.size() * get_trie_evn(list_bfs.size(), density_list_invbfs, density_list_bfs);

  cost_t branching_bfs_novisited = K * (1.0 - bfs_reduction_history.reduced_duplicates);
  cost_t cost_bfs_novisited = branching_bfs_novisited * list_bfs.size() * get_trie_evn(branching_bfs_novisited * list_bfs.size(), density_list_bfs, static_cast<cost_t>(branching_bfs_novisited * card_list_bfs) / (N * (branching_bfs_novisited * list_bfs.size())));
  if (decision.bfs_novisited) branching_bfs_novisited *= (1.0 - bfs_reduction_history.reduced_visited); // TODO: use reduced_self instead of visited
  cost_bfs_novisited += list_invbfs.size() * get_trie_evn(branching_bfs_novisited * list_bfs.size(), density_list_invbfs, density_list_bfs);

  cost_t branching_invbfs_novisited = K * (1.0 - invbfs_reduction_history.reduced_duplicates);
  cost_t cost_invbfs_novisited = branching_invbfs_novisited * list_invbfs.size() * get_trie_evn(branching_invbfs_novisited * list_invbfs.size(), 1.0 - density_list_invbfs, 1.0 - density_list_invbfs);
  branching_invbfs_novisited *= (1.0 - invbfs_reduction_history.reduced_self);
  cost_invbfs_novisited += branching_invbfs_novisited * list_invbfs.size() * get_trie_evn(list_bfs.size(), density_list_invbfs, density_list_bfs);

  int inf_cnt = 0;
  if (decision.bfs_novisited ||
      get_memory_usage() + synchrolib::get_memory_usage(list_bfs_visited) > max_memory || // reduce
      get_memory_usage() + 2 * sizeof(Subset<N>) * list_bfs.size() * K > max_memory ||    // next list x 2
      out_of_memory_dfs(list_bfs.size())) {
    cost_bfs_visited = std::numeric_limits<cost_t>::infinity();
    inf_cnt++;
  }

  if (get_memory_usage() - synchrolib::get_memory_usage(list_bfs_visited) + 2 * sizeof(Subset<N>) * list_bfs.size() * K > max_memory || // next list + reduce
      out_of_memory_dfs(list_bfs.size())) {
    cost_bfs_novisited = std::numeric_limits<cost_t>::infinity();
    inf_cnt++;
  }

  if (decision.invbfs_novisited ||
      get_memory_usage() + synchrolib::get_memory_usage(list_invbfs_visited) > max_memory || // reduce
      get_memory_usage() + 2 * sizeof(Subset<N>) * list_invbfs.size() * K > max_memory ||    // next list x 2
      out_of_memory_dfs(list_invbfs.size())) {
    cost_invbfs_visited = std::numeric_limits<cost_t>::infinity();
    inf_cnt++;
  }

  if (get_memory_usage() - synchrolib::get_memory_usage(list_invbfs_visited) + sizeof(Subset<N>) * list_invbfs.size() * K > max_memory || // next list
      out_of_memory_dfs(list_invbfs.size())) {
    cost_invbfs_novisited = std::numeric_limits<cost_t>::infinity();
    inf_cnt++;
  }

  cost_t branching_invdfs = K * (1.0-invbfs_reduction_history.reduced_duplicates);
  uint64 remaining_iterations = max_reset_threshold - reset_threshold;

  Logger::debug() << "Costs: bfs_visited: " << cost_bfs_visited << " bfs_novisited: " << cost_bfs_novisited << " invbfs_visited: " << cost_invbfs_visited << " invbfs_novisited: " << cost_invbfs_novisited;

  // Predictions

  auto get_dfs_total_factor = [&](const uint depth) {
    return branching_invdfs * (std::pow(branching_invdfs, depth) - 1.0) / (branching_invdfs - 1.0);
  };

  cost_t prediction_bfs_visited = cost_bfs_visited +
      DFS_COST_WEIGHT * list_invbfs.size() * get_dfs_total_factor(remaining_iterations - 1) *
      get_trie_evn(branching_bfs_visited * list_bfs.size(), density_list_invbfs, density_list_bfs);

  cost_t prediction_invbfs_visited = cost_invbfs_visited +
      DFS_COST_WEIGHT * branching_invbfs_visited * list_invbfs.size() * get_dfs_total_factor(remaining_iterations - 1) *
      get_trie_evn(list_bfs.size(), density_list_invbfs, density_list_bfs);

  cost_t prediction_bfs_novisited = cost_bfs_novisited +
      DFS_COST_WEIGHT * list_invbfs.size() * get_dfs_total_factor(remaining_iterations - 1) *
      get_trie_evn(branching_bfs_novisited * list_bfs.size(), density_list_invbfs, density_list_bfs);

  cost_t prediction_invbfs_novisited = cost_invbfs_novisited +
      DFS_COST_WEIGHT * branching_invbfs_novisited * list_invbfs.size() * get_dfs_total_factor(remaining_iterations - 1) *
      get_trie_evn(list_bfs.size(), density_list_invbfs, density_list_bfs);

  cost_t prediction_invdfs =
      DFS_COST_WEIGHT * list_invbfs.size() * get_dfs_total_factor(remaining_iterations) *
      get_trie_evn(list_bfs.size(), density_list_invbfs, density_list_bfs);

  Logger::debug() << "bfs_visited: " << prediction_bfs_visited << " bfs_novisited: " << prediction_bfs_novisited << " invbfs_visited: " << prediction_invbfs_visited << " invbfs_novisited: " << prediction_invbfs_novisited << " invdfs: " << prediction_invdfs;

  cost_t minimum = std::min(std::min(prediction_bfs_visited, prediction_bfs_novisited), std::min(prediction_invbfs_visited, prediction_invbfs_novisited));

  if (inf_cnt == 4) {
    Logger::verbose() << "Ended by memory limit";
    decision.phase = Decision::Phase::IDFS;
    return;
  }

  Logger::debug() << "BFS steps: " << steps_bfs << " IBFS steps: " << steps_invbfs;
#if DFS_SHORTCUT
  if (last_bfs_list_size < list_bfs.size() && last_invbfs_list_size < list_invbfs.size() && prediction_invdfs < minimum) {
    Logger::verbose() << "Ended by shortcut (inf " << inf_cnt << "/4)";
    decision.phase = Decision::Phase::IDFS;
    return;
  }
#endif

  if (prediction_bfs_visited <= prediction_bfs_novisited && prediction_invbfs_visited <= prediction_invbfs_novisited) {
    // If we do not consider novisited, then we choose greedily the cheaper step
    decision.phase = (cost_invbfs_visited < cost_bfs_visited ? Decision::Phase::IBFS : Decision::Phase::BFS);
    if (decision.phase == Decision::Phase::IBFS) Logger::debug() << "Decision: IBFS (greedy)"; else Logger::debug() << "Decision: BFS (greedy)"; 
    return;
  }

  if (minimum == prediction_bfs_visited) {
    Logger::debug() << "Decision: BFS";
    decision.phase = Decision::Phase::BFS;
    return;
  }
  if (minimum == prediction_bfs_novisited) {
    Logger::debug() << "Decision: BFS (novisited)";
    decision.bfs_novisited = true;
    if (!list_bfs_visited.empty()) {
      list_bfs_visited = FastVector<Subset<N>>();
    }
    decision.phase = Decision::Phase::BFS;
    return;
  }
  if (minimum == prediction_invbfs_visited) {
    Logger::debug() << "Decision: IBFS";
    decision.phase = Decision::Phase::IBFS;
    return;
  }
  if (minimum == prediction_invbfs_novisited) {
    Logger::debug() << "Decision: IBFS (novisited)";
    decision.invbfs_novisited = true;
    if (!list_invbfs_visited.empty()) {
      list_invbfs_visited = FastVector<Subset<N>>();
    }
    decision.phase = Decision::Phase::IBFS;
    return;
  }

  throw std::runtime_error("Impossible");
}

template<uint N, uint K>
uint64 MeetInTheMiddle<N, K>::get_cardinalities(const FastVector<Subset<N>>& list) {
  uint64 c = 0;
  for (const auto& sub : list) {
    c += sub.size();
  }
  return c;
}

template<uint N, uint K>
uint64 MeetInTheMiddle<N, K>::get_negated_cardinalities(const FastVector<Subset<N>>& list) {
  uint64 c = 0;
  for (const auto& sub : list) {
    c += sub.size();
  }
  return static_cast<uint64>(N) * list.size() - c;
}

template<uint N, uint K>
double MeetInTheMiddle<N, K>::get_trie_evn(const cost_t m, const cost_t p, const cost_t q) {
  cost_t e = ((1.0 + p) / p + 1.0 / (q - p * q)) *
      std::pow(m, std::log(1.0 + p) / std::log((1.0 + p) / (1.0 + p * q - q)));
  if (e >= 0 && e < m * N) return e;
  return m * N;
}

template<uint N, uint K>
bool MeetInTheMiddle<N, K>::out_of_memory_dfs(size_t list_size) const {
#if !DFS
    return false;
#endif

  size_t additional = list_size * (K - 1) * sizeof(Subset<N>);
  size_t taken = DFS_MIN_LIST_SIZE * sizeof(Subset<N>) * (K + 1);
  if (additional <= taken) {
    return false;
  }

  return DFS_MIN_LIST_SIZE * sizeof(Subset<N>) * (K + 1) * (max_reset_threshold - reset_threshold)
    + additional - taken > max_memory;
}

template class MeetInTheMiddle<AUT_N, AUT_K>;

}  // namespace synchrolib

#ifndef __INTELLISENSE__
$EXACT_UNDEF$
#endif

#endif
