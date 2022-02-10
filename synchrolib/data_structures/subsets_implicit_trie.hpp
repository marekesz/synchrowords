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

template <uint N, bool Proper=false, uint Threads=1, bool SortUniqueDone=false>
class SubsetsImplicitTrie {
public:
  // using Bitset = FastVector<bool>;
  using Bitset = std::vector<bool>; // TODO: change to FastVector (and check memory)

  static Bitset check_contains_subset(FastVector<Subset<N>>& set, FastVector<Subset<N>>& check) {
    if constexpr (!SortUniqueDone) {
      sort_keep_unique(set);
      sort_keep_unique(check);
    }

    if constexpr (Threads == 1 || (GPU && (THREADS == 1))) {
      return check_contains_subset_singlethreaded(set, check);
    } else {
      return check_contains_subset_multithreaded(set, check);
    }
  }

  static void reduce(FastVector<Subset<N>>& set) {
    static_assert(Proper,
      "reduce(FastVector<Subset<N>>&) available only with Proper=true");

    Timer timer("reduce one");
    if (set.empty()) return;

    auto cpy = set; // TODO: somehow opt memory?
    auto remove = check_contains_subset(cpy, set);
    for (int i = static_cast<int>(set.size()) - 1; i >= 0; --i) { // TODO: remove_if
      if (remove[i]) {
        std::swap(set.back(), set[i]);
        set.pop_back();
      }
    }
  }

  static void reduce(FastVector<Subset<N>>& set, FastVector<Subset<N>>& vec) {
    Timer timer("reduce two");
    if (vec.empty()) return;

    auto remove = check_contains_subset(set, vec);
    for (int i = static_cast<int>(vec.size()) - 1; i >= 0; --i) {
      if (remove[i]) {
        std::swap(vec.back(), vec[i]);
        vec.pop_back();
      }
    }
  }

private:
#if (GPU && (THREADS == 1))
  static constexpr uint M = 10000;
#else
  static constexpr uint M = 6;
#endif

  using Iterator = typename FastVector<Subset<N>>::iterator;

  Bitset ret;
#if (GPU && (THREADS == 1))
  SubsetsImplicitTrieKernel<N, Proper> kernel;
  Iterator first_set;
#endif

  static Bitset check_contains_subset_singlethreaded(FastVector<Subset<N>>& set, FastVector<Subset<N>>& check) {
    SubsetsImplicitTrie trie;
    trie.ret = Bitset(check.size());
#if (GPU && (THREADS == 1))
      trie.kernel.allocate(set.data(), set.size(), check.size());
      trie.first_set = set.begin();
#endif
    trie.check_contains_subset_impl(0, 0, set.begin(), set.end(), check.begin(), check.end());
    return trie.ret;
  }

  static Bitset check_contains_subset_multithreaded(FastVector<Subset<N>>& set, FastVector<Subset<N>>& check) {
    Bitset ret(check.size());

    using Range = std::pair<size_t, size_t>;
    size_t set_cnt = std::distance(set.begin(), set.end());
    size_t check_cnt = std::distance(check.begin(), check.end());

    std::array<std::tuple<Range, size_t>, Threads> split;
    for (size_t t = 0; t < Threads; ++t) {
      split[t] = {
        {t * check_cnt / Threads, (t + 1) * check_cnt / Threads}, t
      };
    }

    std::mutex write_mutex;

    static FastVector<std::thread> threads;
    for (size_t t = 0; t < Threads; ++t) {
      threads.push_back(std::thread([] (
          std::tuple<Range, size_t> ranges,
          std::mutex& write_mutex,
          Bitset& ret,
          FastVector<Subset<N>>& set,
          FastVector<Subset<N>>& check) {

        SubsetsImplicitTrie trie;
        trie.ret = Bitset(std::get<0>(ranges).second - std::get<0>(ranges).first);
#if (GPU && (THREADS == 1))
        trie.kernel.allocate(set.data(), set.size(), std::get<0>(ranges).second - std::get<0>(ranges).first);
        trie.first_set = set.begin();
#endif
        trie.check_contains_subset_impl(
            0,
            0,
            set.begin(),
            set.end(),
            check.begin() + std::get<0>(ranges).first,
            check.begin() + std::get<0>(ranges).second);

        const std::lock_guard<std::mutex> lock(write_mutex);
        for (size_t i = std::get<0>(ranges).first; i < std::get<0>(ranges).second; ++i) {
          ret[i] = ret[i] || trie.ret[i - std::get<0>(ranges).first];
        }
      }, split[t], std::ref(write_mutex), std::ref(ret), std::ref(set), std::ref(check)));
    }

    for(auto& thread : threads) {
      thread.join();
    }
    threads.clear();

    return ret;
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
  void check_contains_subset_impl(uint depth, uint ret_pos, Iterator set_begin, Iterator set_end, Iterator check_begin, Iterator check_end) {
    if (set_begin == set_end || check_begin == check_end) return;

    uint set_count = std::distance(set_begin, set_end);
    uint check_count = std::distance(check_begin, check_end);

    if (set_count <= M) { // true for sure if depth == N
#if (GPU && (THREADS == 1))
      Timer timer("gpu");
      // bool show = (std::rand()%50) == 0;
      // bool show = true;
      // if (show) std::cout << set_count << " " << check_count;
      bool* ans = new bool[check_count];
      kernel.run(
        std::distance(first_set, set_begin),
        set_count,
        std::addressof(*check_begin),
        check_count,
        ans);
      uint cnt = 0;
      for (uint i = 0; i < check_count; ++i) {
        if (ans[i]) {
          cnt++;
          ret[ret_pos + i] = true;
        }
      }
      // if (show) std::cout << " | " << cnt << std::endl;
      delete[] ans;
      timer.template stop<false>();
#else
      for (auto set = set_begin; set != set_end; ++set) {
        auto it = check_begin;
        for (uint i = 0; i < check_count; ++i, ++it) {
          if constexpr (Proper) {
            if (it->is_proper_subset(*set)) {
              ret[ret_pos + i] = true;
            }
          } else {
            if (it->is_subset(*set)) {
              ret[ret_pos + i] = true;
            }
          }
        }
      }
#endif
      return;
    }

    auto set_lo = binsearch_first_one(set_begin, depth, set_count);

    if (set_begin != set_lo) {
      check_contains_subset_impl(
        depth + 1,
        ret_pos,
        set_begin,
        set_lo,
        check_begin,
        check_end);
    }

    if (set_lo == set_end) {
      return;
    }

    // deleting already marked elements
    // ================================
    // size_t pos = ret_pos;
    // for (auto it = check_begin; it != check_end; ++it, ++pos) {
    //   if (ret[pos]) {
    //     if (it == check_begin) {
    //       check_begin++;
    //       ret_pos++;
    //     } else {
    //       std::swap(*it, *check_begin);
    //       check_begin++;
    //       ret[ret_pos] = true;
    //       ret[pos] = false;
    //       ret_pos++;
    //     }
    //   }
    // }
    // if (check_begin == check_end) {
    //   return;
    // }
    // ================================

    auto lo = check_begin;
    auto hi = std::prev(check_end);
    while (true) {
      while (lo < hi && !hi->is_set(depth)) {
        hi--;
      }
      while (lo < hi && lo->is_set(depth)) {
        lo++;
      }
      if (lo < hi) {
        std::swap(*lo, *hi);
        bool a = ret[ret_pos + std::distance(check_begin, lo)];
        bool b = ret[ret_pos + std::distance(check_begin, hi)];
        ret[ret_pos + std::distance(check_begin, lo)] = b;
        ret[ret_pos + std::distance(check_begin, hi)] = a;
        lo++;
        hi--;
      } else {
        break;
      }
    }
    if (lo->is_set(depth)) lo++; // lo is the first element with bit set to zero (or end)

    if (check_begin != lo) {
      check_contains_subset_impl(
        depth + 1,
        ret_pos,
        set_lo,
        set_end,
        check_begin,
        lo);
    }
  }
};

}  // namespace synchrolib
