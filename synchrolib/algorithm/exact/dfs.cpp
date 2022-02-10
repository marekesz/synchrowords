#include <jitdefines.hpp>
#ifdef COMPILE_EXACT

#include "dfs.hpp"

#include <algorithm>
#include <synchrolib/algorithm/algorithm.hpp>
#include <synchrolib/algorithm/exact/utils.hpp>
#include <synchrolib/algorithm/exact/meet_in_the_middle.hpp>
#include <synchrolib/data_structures/automaton.hpp>
#include <synchrolib/data_structures/preprocessed_transition.hpp>
#include <synchrolib/data_structures/subset.hpp>
#include <synchrolib/data_structures/subset_utils.hpp>
#include <synchrolib/utils/vector.hpp>
#include <synchrolib/data_structures/subsets_trie.hpp>
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
#include <future>
#include <optional>
#include <utility>

#ifndef __INTELLISENSE__
$EXACT_DEF$
#endif

namespace synchrolib {

template<uint N, uint K>
Dfs<N, K>::Dfs(
    Automaton<N, K>& aut,
    InverseAutomaton<N, K>& invaut,
    std::array<PreprocessedTransition<N, K>, K>& ptrans,
    std::array<PreprocessedTransition<N, K>, K>& invptrans,
    uint64& reset_threshold,
    FastVector<Subset<N>>& list_bfs,
    FastVector<Subset<N>>& list_invbfs,
    uint64 max_depth,
    size_t max_memory):
  aut(aut),
  invaut(invaut),
  ptrans(ptrans),
  invptrans(invptrans),
  reset_threshold(reset_threshold),
  list_bfs(list_bfs),
  list_invbfs(list_invbfs),
  max_depth(max_depth),
  max_memory(max_memory) {
}

template<uint N, uint K>
bool Dfs<N, K>::run() {
  assert(reset_threshold <= max_depth);
  if (reset_threshold == max_depth) {
    return true;
  }

  Logger::verbose() << "Entering dfs";

  try {
    prepare();

    Logger::verbose() << "(DFS) range: [" << reset_threshold << ", " << max_depth << "]"
                      << " trie_bfs sets count: " << trie_bfs.get_sets_count()
                      << " initial list size: " << list_invbfs.size()
                      << " max list size: " << dfs_max_list_size;

    Timer timer_dfs("dfs");
    process_invdfs(0, list_invbfs.size(), reset_threshold, 0);
    timer_dfs.stop();

  } catch (OutOfMemoryException& ex) {
    Logger::error() << "(DFS) out of memory, please consider changing dfs_min_list_size to something smaller";
    return false;
  }

  reset_threshold = max_depth;
  return true;
}

template<uint N, uint K>
size_t Dfs<N, K>::get_memory_usage() const {
  return synchrolib::get_memory_usage(list_bfs) +
      synchrolib::get_memory_usage(list_invbfs) +
      synchrolib::get_memory_usage(trie_bfs) +
      synchrolib::get_memory_usage(ptrans) +
      synchrolib::get_memory_usage(invptrans);
}

template<uint N, uint K>
FastVector<uint> Dfs<N, K>::get_order() {
  FastVector<double> weights(N);
  for (const auto& sub : list_bfs) {
    for (uint i = 0; i < N; ++i) {
      if (sub.is_set(i)) {
        weights[i]++;
      }
    }
  }

  FastVector<std::pair<double, uint>> indices(N);
  for (uint i = 0; i < N; ++i) {
    indices[i] = {
      1.0 - weights[i] / (list_bfs.size() * N),
      i
    };
  }

  std::sort(indices.begin(), indices.end());
  FastVector<uint> order(N);
  for (uint i = 0; i < N; i++) {
    order[indices[i].second] = i;
  }

  return order;
}

template<uint N, uint K>
void Dfs<N, K>::prepare() {
  Logger::verbose() << "Permuting the automaton";
  auto order = get_order();

  for (auto& item : list_bfs) {
    item = Subset<N>::Permutation(item, order);
  }
  for (auto& item : list_invbfs) {
    item = Subset<N>::Permutation(item, order);
  }

  aut = Automaton<N, K>::permutation(aut, order);
  invaut = InverseAutomaton(aut);
  for (uint k = 0; k < K; k++) {
    ptrans[k] = PreprocessedTransition<N, K>(aut, k);
    invptrans[k] = PreprocessedTransition<N, K>(invaut, k);
  }

  Logger::verbose() << "Building trie";
  trie_bfs.template build<true>(std::move(list_bfs)); // TODO: memory check? (should not be needed because of M param)
  list_bfs = FastVector<Subset<N>>();

  update_dfs_max_list_size();
}

template<uint N, uint K>
void Dfs<N, K>::process_invdfs(size_t begin, size_t end, const uint64 lsw, const uint64 depth) {
  size_t initial_size = list_invbfs.size(); // TODO: add list_invbfs.resize(initial_size) as ScopeExit
  size_t size = end - begin;

#if GPU
  static constexpr size_t MAX_REDUCE = 100'000;
#else
  static constexpr size_t MAX_REDUCE = 10'000;
#endif
  size_t reduce_subsets = MAX_REDUCE / std::pow(static_cast<cost_t>(K), depth);

  // TODO: better reduction equations
  auto[found, next_begin, next_end] = invbfs_step_dfs(begin, end,
      (depth <= 3 || depth % 2 == 0) && lsw + 1 < max_depth, // reduce_duplicates
      (depth % 3 == 0 && depth <= 6 && lsw + 10 < max_depth ? 20000 : 0) // old (for benchmark)
      // (lsw + 4 < max_depth && reduce_subsets >= 10 ? reduce_subsets : 0) // new
  );
  if (found) {
    max_depth = lsw;
    if (max_depth > reset_threshold) {
      update_dfs_max_list_size();
    }
    Logger::verbose() << "(DFS) found length: " << (lsw + 1)
                      << " new max depth: " << max_depth
                      << " new max list size " << dfs_max_list_size;
    list_invbfs.resize(initial_size);
    return;
  }

  if (lsw + 1 >= max_depth) {
    list_invbfs.resize(initial_size);
    return;
  }

  size_t next_size = next_end - next_begin;

  size_t partsize =
      (dfs_max_list_size >= size / 2 ? dfs_max_list_size : size / 2);
  Logger::verbose() << "(DFS) length: " << (lsw + 1)
                    << " list size: " << next_size << " branches: "
                    << ((next_size - 1) / partsize + 1)
                    << " memory usage: " << get_megabytes(get_memory_usage());

  size_t pos = 0;
  while (pos + partsize < next_size) {
    process_invdfs(next_begin + pos,
        next_begin + (pos + partsize), lsw + 1, depth + 1);
    if (lsw + 1 >= max_depth) {
      list_invbfs.resize(initial_size);
      return;
    }
    pos += partsize;
    partsize = (dfs_max_list_size >= size / 2 ? dfs_max_list_size : size / 2);
  }
  process_invdfs(next_begin + pos, next_end, lsw + 1, depth + 1);
  list_invbfs.resize(initial_size);
}

template<uint N, uint K>
std::tuple<bool, size_t, size_t> Dfs<N, K>::invbfs_step_dfs(size_t begin, size_t end, bool reduce_duplicates, size_t reduce_subsets) {
  Timer reserve("reserve");
  size_t next_begin = list_invbfs.size();
  list_invbfs.resize(list_invbfs.size() + K * (end - begin));
  size_t next_end = list_invbfs.size();
  reserve.stop();

  Timer timer("apply");
  for (uint k = 0; k < K; k++) {
    invptrans[k].apply(list_invbfs.data() + begin,
      list_invbfs.data() + (next_begin + k * (end - begin)),
      end - begin);
  }
  timer.stop();

  Timer sort_timer("sort & unique");
  sort_sets_cardinality_descending<N>(
      list_invbfs.begin() + next_begin,
      list_invbfs.begin() + next_end,
      [reduce_duplicates] (auto begin, auto end) {
        if (!reduce_duplicates || begin == end) {
          return;
        }

        if constexpr (THREADS == 1) {
          std::sort(begin, end);
        } else {
          parallel_sort(
            std::addressof(*begin),
            std::distance(begin, end),
            std::max(static_cast<size_t>(256), (static_cast<size_t>(std::distance(begin, end)) + THREADS - 1) / THREADS));
        }
      });

  if (reduce_duplicates) {
    next_end = next_begin + std::distance(
      list_invbfs.begin() + next_begin,
      std::unique(list_invbfs.begin() + next_begin, list_invbfs.begin() + next_end));
    list_invbfs.resize(next_end);
  }
  sort_timer.stop();

  if (reduce_subsets) {
    Timer reduce_timer("reduce");
    auto vec = FastVector<Subset<N>>(list_invbfs.begin() + next_begin, list_invbfs.begin() + next_end);

    for (auto& sub : vec) sub.negate();
    auto red = FastVector<Subset<N>>(vec.begin(), vec.begin() + std::min(vec.size(), reduce_subsets));
    Logger::debug() << "red size " << red.size();
    Logger::debug() << "before " << vec.size();
    SubsetsImplicitTrie<N, true, THREADS>::reduce(red, vec);
    Logger::debug() << "after " << vec.size();
    for (auto& sub : vec) sub.negate();

    std::copy(vec.begin(), vec.end(), list_invbfs.begin() + next_begin);
    next_end = next_begin + vec.size();
    list_invbfs.resize(next_end);

    reduce_timer.stop();
  }

  Timer timer_sc("subsets check");
  if constexpr (THREADS > 1) {
    return {check_nextlist_multithreaded(next_begin, next_end), next_begin, next_end};
  }

  for (uint i = next_begin; i < next_end; ++i) {
    if (trie_bfs.contains_subset_of(list_invbfs[i])) {
      return {true, next_begin, next_end};
    }
  }
  return {false, next_begin, next_end};
}

template<uint N, uint K>
bool Dfs<N, K>::check_nextlist_multithreaded(size_t begin, size_t end) {
  bool ret = false;
  std::mutex write_mutex;
  FastVector<std::thread> threads;
  for (size_t t = 0; t < THREADS; ++t) {
      threads.push_back(std::thread([] (
        size_t t,
        std::mutex& write_mutex,
        bool& ret,
        FastVector<Subset<N>>& list_invbfs,
        size_t begin,
        size_t end,
        const SubsetsTrie<N, THREADS>& trie_bfs) {

      bool ans = false;
      for (size_t i = begin + t; i < end; i += THREADS) {
        if (trie_bfs.contains_subset_of(list_invbfs[i])) {
          ans = true;
          break;
        }
      }

      const std::lock_guard<std::mutex> lock(write_mutex);
      ret |= ans;
    }, t, std::ref(write_mutex), std::ref(ret), std::ref(list_invbfs), begin, end, std::ref(trie_bfs)));
  }

  for(auto& thread : threads) {
    thread.join();
  }

  return ret;
}

template<uint N, uint K>
void Dfs<N, K>::update_dfs_max_list_size() {
  auto memory_usage = get_memory_usage();
  if (max_memory < memory_usage) {
#if STRICT_MEMORY_LIMIT
    throw OutOfMemoryException();
#else
    Logger::warning() << "Memory limit reached: " << get_megabytes(memory_usage);
#endif
  }
  memory_usage -= synchrolib::get_memory_usage(list_invbfs);

  dfs_max_list_size = static_cast<size_t>((max_memory - memory_usage) /
      (sizeof(Subset<N>) * (K + 1) * (max_depth - reset_threshold)));

  if (dfs_max_list_size < DFS_MIN_LIST_SIZE) {
#if STRICT_MEMORY_LIMIT
    throw OutOfMemoryException();
#else
    Logger::warning() << "Dfs max list size set to " << DFS_MIN_LIST_SIZE
      << ", which is higher than proposed " << dfs_max_list_size;
#endif
    dfs_max_list_size = DFS_MIN_LIST_SIZE;
  }
}

template class Dfs<AUT_N, AUT_K>;

}  // namespace synchrolib

#ifndef __INTELLISENSE__
$EXACT_UNDEF$
#endif


#endif