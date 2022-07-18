#pragma once
#include <iostream>
#include <vector>
#include <mutex>
#include <limits>
#include <thread>
#include <vector>
#include <memory>

#include <synchrolib/data_structures/automaton.hpp>
#include <synchrolib/data_structures/subset.hpp>
#include <synchrolib/utils/general.hpp>
#include <synchrolib/utils/vector.hpp>
#include <synchrolib/utils/memory.hpp>
#include <synchrolib/utils/timer.hpp>

// TODO: use GPU_MEMORY
#if (GPU && (THREADS == 1))
#include <synchrolib/data_structures/cuda/subsets_implicit_trie_kernel.hpp>
#endif

namespace synchrolib {

template <uint N, bool Proper=false, uint Threads=1, bool SortUniqueDone=false, bool ThreadShuffle=false>
class SubsetsImplicitTrie {
public:
  using Iterator = typename FastVector<Subset<N>>::iterator;

  static Iterator check_contains_subset(FastVector<Subset<N>>& set, FastVector<Subset<N>>& check) {
    if constexpr (!SortUniqueDone) {
      sort_keep_unique(set);
      sort_keep_unique(check);
    }

    if constexpr (Threads == 1 || (GPU && (THREADS == 1))) {
      return check_contains_subset_singlethreaded(set, check);
    } else {
      if (std::max(set.size(), check.size()) < 256) {
        return check_contains_subset_singlethreaded(set, check);
      } else {
        return check_contains_subset_multithreaded(set, check);
      }
    }
  }

  static void reduce(FastVector<Subset<N>>& set) {
    static_assert(Proper,
      "reduce(FastVector<Subset<N>>&) available only with Proper=true");

    Timer timer("reduce one");
    if (set.empty()) return;

    auto cpy = set; // TODO: somehow opt memory?
    auto it = check_contains_subset(cpy, set);
    set.resize(std::distance(set.begin(), it));
  }

  static void reduce(FastVector<Subset<N>>& set, FastVector<Subset<N>>& vec) {
    Timer timer("reduce two");
    if (vec.empty()) return;

    auto it = check_contains_subset(set, vec);
    vec.resize(std::distance(vec.begin(), it));
  }

private:
#if (GPU && (THREADS == 1))
  static constexpr uint M = 10000;
#else
  static constexpr uint M = 6;
#endif

#if (GPU && (THREADS == 1))
  SubsetsImplicitTrieKernel<N, Proper> kernel;
  Iterator first_set;
  FastVector<bool> kernel_ans;
#endif

  static Iterator check_contains_subset_singlethreaded(FastVector<Subset<N>>& set, FastVector<Subset<N>>& check) {
    SubsetsImplicitTrie trie;
#if (GPU && (THREADS == 1))
      trie.kernel.allocate(set.data(), set.size(), check.size());
      trie.first_set = set.begin();
#endif
    auto it = check.end();
    trie.check_contains_subset_impl(0, set.begin(), set.end(), check.begin(), it);
    return it;
  }

  static Iterator check_contains_subset_multithreaded(FastVector<Subset<N>>& set, FastVector<Subset<N>>& check) {
    using Range = std::pair<size_t, size_t>;
    size_t set_cnt = std::distance(set.begin(), set.end());
    size_t check_cnt = std::distance(check.begin(), check.end());

    std::array<std::tuple<Range, size_t>, Threads> split;
    std::array<std::pair<Iterator, Iterator>, Threads> ret;
    for (size_t t = 0; t < Threads; ++t) {
      split[t] = {
        {t * check_cnt / Threads, (t + 1) * check_cnt / Threads}, t
      };
    }

    if constexpr (ThreadShuffle) {
      std::random_shuffle(check.begin(), check.end());
    }

    static FastVector<std::thread> threads;
    for (size_t t = 0; t < Threads; ++t) {
      threads.push_back(std::thread([] (
          uint t,
          std::tuple<Range, size_t> ranges,
          std::array<std::pair<Iterator, Iterator>, Threads>& ret,
          FastVector<Subset<N>>& set,
          FastVector<Subset<N>>& check) {

        SubsetsImplicitTrie trie;
        auto begin = check.begin() + std::get<0>(ranges).first;
        auto end = check.begin() + std::get<0>(ranges).second;
        auto it = end;
        if constexpr (ThreadShuffle) {
          std::sort(begin, end);
        }
        trie.check_contains_subset_impl(
            0,
            set.begin(),
            set.end(),
            begin,
            it);

        ret[t] = {begin, it};
      }, t, split[t], std::ref(ret), std::ref(set), std::ref(check)));
    }

    for(auto& thread : threads) {
      thread.join();
    }
    threads.clear();

    auto it = check.begin();
    for (size_t t = 0; t < Threads; ++t) {
      if (ret[t].first != it) {
        it = std::copy(ret[t].first, ret[t].second, it);
      } else {
        it = ret[t].second;
      }
    }
    return it;
  }

  // binary search first subset with <depth> bit set to one
  static Iterator binsearch_first_one(Iterator begin, uint depth, uint count) {
    while (count > 0) {
      auto step = count / 2;
      auto it = std::next(begin, step);
      if (!it->is_set(depth)) {
        begin = ++it;
        count -= step + 1;
      } else {
        count = step;
      }
    }
    return begin;
  }

  // it's important that there are no duplicates between begin and end
  void check_contains_subset_impl(uint depth, Iterator set_begin, Iterator set_end, Iterator check_begin, Iterator& check_end) {
    if (set_begin == set_end || check_begin == check_end) return;

    uint set_count = std::distance(set_begin, set_end);
    uint check_count = std::distance(check_begin, check_end);

    if (set_count <= M) { // true for sure if depth == N
#if (GPU && (THREADS == 1))
      // Timer timer("gpu");
      // bool show = (std::rand()%50) == 0;
      // bool show = true;
      // if (show) std::cout << set_count << " " << check_count;
      kernel_ans.resize(check_count);
      kernel.run(
        std::distance(first_set, set_begin),
        set_count,
        std::addressof(*check_begin),
        check_count,
        kernel_ans.data());
      auto it = check_end;
      for (int i = static_cast<int>(check_count) - 1; i >= 0; --i) {
        --it;
        if (kernel_ans[i]) {
          --check_end;
          std::swap(*it, *check_end);
        }
      }
      // if (show) std::cout << " | " << cnt << std::endl;
      // timer.template stop<false>();
#else
      for (auto set = set_begin; set != set_end; ++set) {
        auto it = check_begin;
        while (it != check_end) {
          if constexpr (Proper) {
            if (it->is_proper_subset(*set)) {
              --check_end;
              std::swap(*it, *check_end);
            } else {
              ++it;
            }
          } else {
            if (it->is_subset(*set)) {
              --check_end;
              std::swap(*it, *check_end);
            } else {
              ++it;
            }
          }
        }
      }
#endif
      return;
    }

    if (check_begin == check_end) {
      return;
    }

    auto set_lo = binsearch_first_one(set_begin, depth, set_count);

    if (set_begin != set_lo) {
      check_contains_subset_impl(
        depth + 1,
        set_begin,
        set_lo,
        check_begin,
        check_end);
    }

    if (set_lo == set_end || check_begin == check_end) {
      return;
    }

    auto lo = check_begin;
    auto hi = std::prev(check_end);
    while (true) {
      while (lo < hi && hi->is_set(depth)) {
        hi--;
      }
      while (lo < hi && !lo->is_set(depth)) {
        lo++;
      }
      if (lo < hi) {
        std::swap(*lo, *hi);
        lo++;
        hi--;
      } else {
        break;
      }
    }
    if (!lo->is_set(depth)) lo++; // lo is the first element with bit set to one (or end)

    if (lo != check_end) {
      check_contains_subset_impl(
        depth + 1,
        set_lo,
        set_end,
        lo,
        check_end);
    }
  }
};

}  // namespace synchrolib
