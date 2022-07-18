#pragma once
#include <iostream>
#include <vector>
#include <limits>
#include <mutex>
#include <thread>
#include <memory>

#include <synchrolib/data_structures/automaton.hpp>
#include <synchrolib/data_structures/subset.hpp>
#include <synchrolib/utils/general.hpp>
#include <synchrolib/utils/memory.hpp>
#include <synchrolib/utils/timer.hpp>
#include <synchrolib/utils/vector.hpp>

namespace synchrolib {

template <uint N, uint Threads>
class SubsetsTrie : public MemoryUsage {
public:

  struct Node {
    uint zero, one; // it'ss possible to change zero from uint to bool - "is there a zero node?"
    uint division_bit;

    uint subsets_cnt;
    uint subsets_pos;

    uint subtree_min_popcount;
    
    Node():
        zero(0),
        one(0),
        division_bit(N),
        subsets_cnt(0),
        subsets_pos(0),
        subtree_min_popcount(std::numeric_limits<uint>::max()) {}
  };

  using Iterator = typename FastVector<Subset<N>>::iterator;

  FastVector<Node> nodes;
  FastVector<Subset<N>> subsets;

  size_t get_memory_usage() const override {
    return synchrolib::get_memory_usage(nodes)
      + synchrolib::get_memory_usage(subsets);
  }

  SubsetsTrie() { reset(); }

  void reset() {
    nodes = FastVector<Node>(1);
    subsets = FastVector<Subset<N>>();
  }

  template <bool Swap=false>
  void build(FastVector<Subset<N>> vec) {
    Timer timer("build");
    reset();
    subsets = std::move(vec);
    sort_keep_unique(subsets);
    subsets.shrink_to_fit();
    if constexpr (Swap) {
      build_impl_swap(0, 0, subsets.begin(), subsets.end());
    } else {
      build_impl(0, 0, subsets.begin(), subsets.end());
    }
    nodes.shrink_to_fit();
  }

  // template <bool Proper=false>
  // bool contains_subset_of(const Subset<N>& set) const {
  //   return contains_subset_of_impl<Proper>(root(), set);
  // }

  template <bool Proper=false>
  bool contains_subset_of(Iterator begin, Iterator end) const {
    // for (auto it = begin; it != end; ++it) {
    //   if (contains_subset_of(*it)) {
    //     return true;
    //   }
    // }
    // return false;
    if (begin == end) {
      return false;
    }
    uint set_size = begin->size();
    return contains_subset_of_impl<Proper>(root(), begin, end, set_size);
  }

  void get_sets_list(FastVector<Subset<N>> &vec) const {
    for (const auto& sub : subsets) {
      vec.push_back(sub);
    }
  }

  uint64 get_sets_count() const {
    return subsets.size();
  }

  void get_cardinalities(uint64& sets_count, uint64& card) const {
    sets_count = subsets.size();
    card = 0;
    for (const auto& sub : subsets) {
      card += sub.size();
    }
  }

  void get_negated_sets_cardinalities(uint64& sets_count, uint64& card) const {
    sets_count = card = 0;
    get_cardinalities(sets_count, card);
    card = sets_count * N - card;
  }

  // TODO: opt memory
  void reduce() {
    Timer timer("reduce");
    FastVector<Subset<N>> vec;

    Timer timer3("reduce->loop");
    for (const auto& set : subsets) {
      if (!contains_subset_of<true>(set)) {
        vec.push_back(set);
      }
    }
    timer3.stop();

    build(vec);
  }

  // TODO: opt memory
  template <bool Proper=false>
  void reduce(FastVector<Subset<N>>& vec) {
    Timer timer("reduce list");
    FastVector<Subset<N>> new_vec;
    new_vec.reserve(vec.size());

    for (const auto& set : vec) {
      if (!contains_subset_of<Proper>(set)) {
        new_vec.push_back(set);
      }
    }

    new_vec.shrink_to_fit();
    vec = std::move(new_vec);
  }

private:
  static constexpr uint M = 10;

  Node& root() {
    return nodes[0];
  }

  const Node& root() const {
    return nodes[0];
  }

  uint create_node() {
    nodes.emplace_back();
    return nodes.size() - 1;
  }

  // it's important that there are no duplicates between begin and end
  void build_impl(uint v, int depth, Iterator begin, Iterator end) {
    if (begin == end) return;

    uint count = std::distance(begin, end);
    if (count <= M) { // true for sure if depth == N
      nodes[v].subsets_pos = std::distance(subsets.begin(), begin);
      while (begin != end) {
        nodes[v].subsets_cnt++;
        nodes[v].subtree_min_popcount = std::min(nodes[v].subtree_min_popcount, begin->size());
        ++begin;
      }
      return;
    }

    while (begin->is_set(depth) == std::prev(end)->is_set(depth)) {
      depth++;
    }
    nodes[v].division_bit = depth;

    // binary search first subset with <depth> bit set to one
    auto lo = begin;
    while (count > 0) {
      auto step = count / 2;
      auto it = std::next(lo, step);
      if (!it->is_set(depth)) {
        lo = ++it;
        count -= step + 1;
      } else {
        count = step;
      }
    }

    if (std::distance(begin, lo) <= M) {
      nodes[v].subsets_pos = std::distance(subsets.begin(), begin);
      while (begin != end && nodes[v].subsets_cnt < M) {
        nodes[v].subsets_cnt++;
        nodes[v].subtree_min_popcount = std::min(nodes[v].subtree_min_popcount, begin->size());
        ++begin;
      }
      if (lo < begin) {
        lo = begin;
      }
    }
    // TODO: maybe it's a good idea to place ones if we can remove the one link (probably not)
    // nodes[v].subsets_pos = std::distance(subsets.begin(), begin);
    // while (begin != end && nodes[v].subsets_cnt < M) {
    //   nodes[v].subsets_cnt++;
    //   nodes[v].subtree_min_popcount = std::min(nodes[v].subtree_min_popcount, begin->size());
    //   nodes[v].subtree_and &= *begin;
    //   ++begin;
    // }
    // if (lo < begin) {
    //   lo = begin;
    // }

    if (begin != lo) {
      // path of zeros is close in memory
      nodes[v].zero = create_node();
      build_impl(nodes[v].zero, depth + 1, begin, lo);
      nodes[v].subtree_min_popcount = std::min(nodes[v].subtree_min_popcount, nodes[nodes[v].zero].subtree_min_popcount);
    }

    if (lo != end) {
      nodes[v].one = create_node();
      build_impl(nodes[v].one, depth + 1, lo, end);
      nodes[v].subtree_min_popcount = std::min(nodes[v].subtree_min_popcount, nodes[nodes[v].one].subtree_min_popcount);
    }
  }

  // it's important that there are no duplicates between begin and end
  void build_impl_swap(uint v, int depth, Iterator begin, Iterator end) {
    if (begin == end) return;

    size_t all = std::distance(begin, end);
    if (all <= M) { // true for sure if depth == N
      nodes[v].subsets_pos = std::distance(subsets.begin(), begin);
      while (begin != end) {
        nodes[v].subsets_cnt++;
        nodes[v].subtree_min_popcount = std::min(nodes[v].subtree_min_popcount, begin->size());
        ++begin;
      }
      return;
    }

    uint division_bit = get_division_bit(begin, end);
    nodes[v].division_bit = division_bit;

    auto lo = begin;
    auto hi = std::prev(end);
    while (true) {
      while (lo < hi && hi->is_set(division_bit)) {
        hi--;
      }
      while (lo < hi && !lo->is_set(division_bit)) {
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
    if (!lo->is_set(division_bit)) lo++; // lo is the first element with bit set to one (or end)

    // subsets_cnt > 0 only if there is no zero child
    if (std::distance(begin, lo) <= M) {
      nodes[v].subsets_pos = std::distance(subsets.begin(), begin);
      while (begin != end && nodes[v].subsets_cnt < M) {
        nodes[v].subsets_cnt++;
        nodes[v].subtree_min_popcount = std::min(nodes[v].subtree_min_popcount, begin->size());
        ++begin;
      }
      if (lo < begin) {
        lo = begin;
      }
    }

    if (begin != lo) {
      // path of zeros is close in memory
      nodes[v].zero = create_node();
      build_impl_swap(nodes[v].zero, depth + 1, begin, lo);
      nodes[v].subtree_min_popcount = std::min(nodes[v].subtree_min_popcount, nodes[nodes[v].zero].subtree_min_popcount);
    }

    if (lo != end) {
      nodes[v].one = create_node();
      build_impl_swap(nodes[v].one, depth + 1, lo, end);
      nodes[v].subtree_min_popcount = std::min(nodes[v].subtree_min_popcount, nodes[nodes[v].one].subtree_min_popcount);
    }
  }

  inline uint get_division_bit(Iterator begin, Iterator end) {
    size_t all = std::distance(begin, end);
    int count[N] = {};

    size_t thread_cnt = std::min(static_cast<size_t>(Threads), all / 350 + 1);
    if (thread_cnt == 1) {
      for (auto it = begin; it != end; ++it) {
        it->count(count);
      }
    } else {
      using Range = std::pair<size_t, size_t>;

      std::array<std::tuple<Range, size_t>, Threads> split;
      for (size_t t = 0; t < thread_cnt; ++t) {
        split[t] = {
          {t * all / thread_cnt, (t + 1) * all / thread_cnt}, t
        };
      }

      std::mutex write_mutex;
      static FastVector<std::thread> threads;
      for (size_t t = 0; t < thread_cnt; ++t) {
        threads.push_back(std::thread([] (
            std::tuple<Range, size_t> range,
            std::mutex& write_mutex,
            int count[],
            Iterator& begin) {

          int cnt[N] = {};
          auto end = begin + std::get<0>(range).second;
          for (auto it = begin + std::get<0>(range).first; it != end; ++it) {
            it->count(cnt);
          }

          const std::lock_guard<std::mutex> lock(write_mutex);
          for (uint i = 0; i < N; ++i) {
            count[i] += cnt[i];
          }
        }, split[t], std::ref(write_mutex), count, std::ref(begin)));
      }

      for(auto& thread : threads) {
        thread.join();
      }
      threads.clear();
    }

    for (uint i = 0; i < N; ++i) {
      if (count[i] == static_cast<int>(all) || count[i] == 0) count[i] = -1; // do not want all ones or all zeros
    }
    return std::distance(count, std::max_element(count, count + N));
  }

  // TODO: removing maskelim seems to speed up the algorithm
  template <bool Proper>
  bool contains_subset_of_impl(const Node& node, Iterator begin, Iterator end, uint set_size) const {
    assert(!node.zero || node.one);

    if constexpr (Proper) {
      if (set_size <= node.subtree_min_popcount) {
        return false;
      }
    } else {
      if (set_size < node.subtree_min_popcount) {
        return false;
      }
    }

    auto it = begin;
    if (node.zero) {
      // while (it != end) {
      //   if constexpr (Proper) {
      //     if (!it->is_proper_subset(node.subtree_and)) {
      //       --end;
      //       std::swap(*it, *end);
      //     } else {
      //       ++it;
      //     }
      //   } else {
      //     if (!it->is_subset(node.subtree_and)) {
      //       --end;
      //       std::swap(*it, *end);
      //     } else {
      //       ++it;
      //     }
      //   }
      // }
      // if (begin == end) {
      //   return false;
      // }
      if (contains_subset_of_impl<Proper>(nodes[node.zero], begin, end, set_size)) {
        return true;
      }
    } else if (node.one) {
      while (it != end) {
        if constexpr (Proper) {
          // if (!it->is_proper_subset(node.subtree_and)) {
          //   --end;
          //   std::swap(*it, *end);
          // } else {
            for (const Subset<N> *s = &subsets[node.subsets_pos]; s != &subsets[node.subsets_pos+node.subsets_cnt]; ++s) {
              if (it->is_proper_subset(*s)) {
                return true;
              }
            }
            ++it;
          // }
        } else {
          // if (!it->is_subset(node.subtree_and)) {
          //   --end;
          //   std::swap(*it, *end);
          // } else {
            for (const Subset<N> *s = &subsets[node.subsets_pos]; s != &subsets[node.subsets_pos+node.subsets_cnt]; ++s) {
              if (it->is_subset(*s)) {
                return true;
              }
            }
            ++it;
          // }
        }
      }
      // if (begin == end) {
      //   return false;
      // }
    } else {
      while (it != end) {
        if constexpr (Proper) {
          // if (it->is_proper_subset(node.subtree_and)) {
            for (const Subset<N> *s = &subsets[node.subsets_pos]; s != &subsets[node.subsets_pos+node.subsets_cnt]; ++s) {
              if (it->is_proper_subset(*s)) {
                return true;
              }
            }
          // }
        } else {
          // if (it->is_subset(node.subtree_and)) {
            for (const Subset<N> *s = &subsets[node.subsets_pos]; s != &subsets[node.subsets_pos+node.subsets_cnt]; ++s) {
              if (it->is_subset(*s)) {
                return true;
              }
            }
          // }
        }
        ++it;
      }
      return false;
    }

    auto bit = node.division_bit;
    auto lo = begin;
    auto hi = std::prev(end);
    while (true) {
      while (hi->is_set(bit)) {
        if (lo >= hi) goto lb_endwhile;
        hi--;
      }
      while (!lo->is_set(bit)) {
        if (lo >= hi) goto lb_endwhile;
        lo++;
      }
      std::swap(*lo, *hi);
      lo++;
      hi--;
      if (lo >= hi) goto lb_endwhile;
    }
    lb_endwhile:;
    if (!lo->is_set(bit)) lo++; // lo is the first element with bit set to one (or end)
    
    return (lo != end) && contains_subset_of_impl<Proper>(nodes[node.one], lo, end, set_size);
  }

  // TODO: make popcount static
  // template <bool Proper>
  // bool contains_subset_of_impl(const Node& node, const Subset<N>& set) const {
  //   assert(!node.zero || node.one);

  //   if constexpr (Proper) {
  //     if (set.size() <= node.subtree_min_popcount) {
  //       return false;
  //     }
  //     // if (!set.is_proper_subset(node.subtree_and)) {
  //     //   return false;
  //     // }
  //   } else {
  //     if (set.size() < node.subtree_min_popcount) {
  //       return false;
  //     }
  //     // if (!set.is_subset(node.subtree_and)) {
  //     //   return false;
  //     // }
  //   }
    
  //   assert(node.division_bit < N || (!node.zero && !node.one));

  //   if (node.zero) {
  //     if (!set.is_set(node.division_bit)) { // opt for quick return
  //       return contains_subset_of_impl<Proper>(nodes[node.zero], set);
  //     }
  //     if (contains_subset_of_impl<Proper>(nodes[node.zero], set)) {
  //       return true;
  //     }
  //     return contains_subset_of_impl<Proper>(nodes[node.one], set);
  //   }

  //   for (const Subset<N> *s = &subsets[node.subsets_pos]; s != &subsets[node.subsets_pos+node.subsets_cnt]; s++) {
  //     if constexpr (Proper) {
  //       if (set.is_proper_subset(*s)) {
  //         return true;
  //       }
  //     } else {
  //       if (set.is_subset(*s)) {
  //         return true;
  //       }
  //     }
  //   }

  //   return node.one && set.is_set(node.division_bit) && contains_subset_of_impl<Proper>(nodes[node.one], set);
  // }

};

}  // namespace synchrolib
