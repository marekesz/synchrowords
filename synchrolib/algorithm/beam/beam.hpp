#pragma once
#include <algorithm>
#include <synchrolib/algorithm/algorithm.hpp>
#include <synchrolib/data_structures/automaton.hpp>
#include <synchrolib/data_structures/sets_trie_old.hpp>
#include <synchrolib/data_structures/subset.hpp>
#include <synchrolib/data_structures/preprocessed_transition.hpp>
#include <synchrolib/data_structures/subset_utils.hpp>
#include <synchrolib/utils/connectivity.hpp>
#include <synchrolib/utils/vector.hpp>
#include <synchrolib/utils/general.hpp>
#include <synchrolib/utils/logger.hpp>
#include <synchrolib/utils/distribution.hpp>
#include <cassert>
#include <cmath>
#include <cstring>
#include <numeric>
#include <optional>
#include <utility>
#include <vector>

#ifndef __INTELLISENSE__
$BEAM_DEF$
#endif

namespace synchrolib {

template <uint N, uint K>
class Beam : public Algorithm<N, K> {
public:
  void run(AlgoData<N, K>& data) override {
    auto log_name = Logger::set_log_name("beam");

    if (data.result.non_synchro) {
      return;
    }

    if (data.result.mlsw_lower_bound == data.result.mlsw_upper_bound) {
      return;
    }

    if (N == 1) {
      data.result.mlsw_upper_bound = data.result.mlsw_lower_bound = 0;
      Logger::info() << "Upper bound: " << data.result.mlsw_upper_bound;
      return;
    }

    if (data.result.reduce && data.result.reduce->done) {
      Logger::error()
          << "Algorithm is not compatible with inputs after reduction";
      return;
    }

#ifdef PRESORT_NONE
    auto perm = get_identity_permutation();
#endif

#ifdef PRESORT_INDEG
    auto perm = get_indeg_permutation(data.aut);
#endif

    auto permuted_aut = Automaton<N, K>::permutation(data.aut, perm);
    auto permuted_invaut = InverseAutomaton<N, K>(permuted_aut);

    uint bound;
    if constexpr (DYNAMIC) {
      using cost_t = long double;

      constexpr cost_t beam_cost_weight = BEAM_EXACT_RATIO;
      constexpr uint minimal_beamsize = MIN_BEAM_SIZE;
      constexpr uint maximal_beamsize = MAX_BEAM_SIZE;

      bound = get_automaton_lsw_cutoffinvbfs(permuted_aut, permuted_invaut, minimal_beamsize, data.result.mlsw_upper_bound);

      cost_t exact_cost = 2 * std::pow(static_cast<cost_t>(K), (bound + 1) / 2);
      cost_t proposed_beamsize = std::round(exact_cost * beam_cost_weight / bound);
      uint beamsize = std::clamp<cost_t>(proposed_beamsize, minimal_beamsize, maximal_beamsize);
      Logger::verbose() << "Dynamic beam: exact_cost " << exact_cost
        << " proposed_beamsize " << proposed_beamsize
        << " beamsize " << beamsize
        << " min beamsize " << minimal_beamsize
        << " max beamsize " << maximal_beamsize;

      if (beamsize > minimal_beamsize) {
        bound = get_automaton_lsw_cutoffinvbfs(permuted_aut, permuted_invaut, beamsize, bound);
      }
    } else {
      bound = get_automaton_lsw_cutoffinvbfs(permuted_aut, permuted_invaut, BEAM_SIZE, data.result.mlsw_upper_bound);
    }

    data.result.mlsw_upper_bound = bound;
    Logger::info() << "Upper bound: " << data.result.mlsw_upper_bound;

    assert(data.result.mlsw_lower_bound <= data.result.mlsw_upper_bound);
  }

private:
  uint64 get_automaton_lsw_cutoffinvbfs(const Automaton<N, K>& aut,
      const InverseAutomaton<N, K>& invaut, const uint beam_size,
      const uint64 max_mlsw) {
    FastVector<Subset<N>> list;
    FastVector<Subset<N>> list_next;

    FastVector<bool> sink;
    get_automaton_reachable_states(aut, get_automaton_sink_component_vertex(aut), sink);
    for (uint n = 0; n < N; n++) {
      if (!sink[n]) continue;

      for (uint k = 0; k < K; k++) {
        if (invaut.end(n, k) - invaut.begin(n, k) > 1) {
          list.push_back(Subset<N>::Singleton(n));
          break;
        }
      }
    }

    std::array<PreprocessedTransition<N, K>, K> invptrans;
    for (uint k = 0; k < K; k++) {
      invptrans[k] = PreprocessedTransition<N, K>(invaut, k);
    }

    size_t apply_time = 0;
    size_t sort_time = 0;

    for (uint64 len = 1; len < max_mlsw; len++) {
      if constexpr (MAX_ITER >= 0) {
        if (len > MAX_ITER) {
          Logger::info() << "max_iter iterations completed, exiting...";
          break;
        }
      }

      Logger::debug() << "depth: " << len << " list size: " << list.size();
      list_next.resize(K * list.size());

      Timer apply_timer("apply");
      if constexpr (true) {
        for (uint k = 0; k < K; k++) {
          invptrans[k].apply(list.data(),
            list_next.data() + (k * list.size()),
            list.size());
        }
      } else {
        for (size_t i = 0; i < list.size(); ++i) {
          size_t offset = i * K;
          for (uint k = 0; k < K; ++k) {
            list[i].invapply(invaut, k, list_next[offset + k]);
            auto size = list_next[offset + k].size();
            if (size == N) {
              return len;
            }
          }
        }
      }
      apply_time += apply_timer.stop<false, std::chrono::microseconds>();

      std::swap(list, list_next);

      Timer sort_timer("sort");
      sort_sets_cardinality_descending<N>(list.begin(), list.end(), [&](auto begin, auto end, uint card){
        if constexpr (true || THREADS == 1) { // TODO: use parallel
          std::sort(begin, end, Subset<N>::comp_fast);
        } else {
          parallel_sort(
            std::addressof(*begin),
            std::distance(begin, end),
            std::max(static_cast<size_t>(1024), (static_cast<size_t>(std::distance(begin, end)) + THREADS - 1) / THREADS),
            Subset<N>::comp_fast);
        }
      });
      sort_time += sort_timer.stop<false, std::chrono::microseconds>();

      if (list[0].size() == N) {
        Logger::debug() << "apply sum " << apply_time / 1000 << "ms";
        Logger::debug() << "sort sum " << sort_time / 1000 << "ms";
        return len;
      }

      keep_unique(list);
      if (list.size() > beam_size) {
        list.resize(beam_size);
      }

      Logger::debug() << list[0].size();
    }

    return max_mlsw;
  }

  std::array<double, N> compute_scores_avgindeg2(const Automaton<N,K> &aut) {
    std::array<double, N> indegs{};
    for (uint n=0;n<N;n++) {
      for (uint k1=0;k1<K;k1++) {
        for (uint k2=0;k2<K;k2++) {
          indegs[aut[aut[n][k1]][k2]]++;
        }
      }
    }

    std::array<double, N> scores;
    for (uint n=0;n<N;n++) {
      scores[n] = 1.0/(static_cast<double>(K)*K*N*N) + indegs[n];
    }
    return scores;
  }

  std::array<double, N> compute_scores_distribution(const Automaton<N,K> &aut) {
    std::array<double, N> scores;
    FastVector<double> pi(N);
    get_automaton_stationary_distribution(aut, pi);
    for (uint n = 0; n < N; n++) scores[n] = pi[n]*N;
    return scores;
  }

  struct SetsWeightsComparator {
    SetsWeightsComparator(std::array<double, N> weights_): weights(weights_) {}

    std::array<double, N> weights;

    double get_weight(const Subset<N> &s) {
      double w = 0.0;
      for (uint n=0;n<N;n++) if (s.is_set(n)) w += weights[n];
      return w;
    }

    bool operator() (const Subset<N> &s1, const Subset<N> &s2) {
      return get_weight(s1) > get_weight(s2);
    }
  };

  uint64 get_automaton_lsw_cutoffinvbfs_scores(const Automaton<N, K>& aut,
      const InverseAutomaton<N, K>& invaut, const uint beam_size,
      const uint64 max_mlsw) {
    // auto comparator = SetsWeightsComparator(compute_scores_avgindeg2(aut));
    auto comparator = SetsWeightsComparator(compute_scores_distribution(aut));
    const uint CK = K * (N > beam_size ? N : beam_size);
    SetsTrie<N> trie_invbfs;
    FastVector<Subset<N>> list(CK);
    uint size = 0;
    {
      FastVector<bool> sink;
      get_automaton_reachable_states(aut, get_automaton_sink_component_vertex<N, K>(aut), sink);

      for (uint n = 0; n < N; n++) {
        if (sink[n]) {
          for (uint k = 0; k < K; k++) {
            if (invaut.end(n, k) - invaut.begin(n, k) > 1) {
              list[size++] = Subset<N>::Singleton(n);
              break;
            }
          }
        }
      }
    }

    for (uint len=1;len<max_mlsw;len++) {
      if constexpr (MAX_ITER >= 0) {
        if (len > MAX_ITER) {
          Logger::info() << "max_iter iterations completed, exiting...";
          break;
        }
      }

      Logger::verbose() << "depth: " << len << " list size: " << size;

      // IBFS step
      for (size_t i=0;i<size;i++) {
        for (uint k=0;k<K;k++) {
          Subset<N> s;
          list[i].template invapply<N,K>(invaut, k, s);
          uint subset_size = s.size();
          if (subset_size <= 1) continue;
          if (subset_size == N){
            return len;
          }
          trie_invbfs.insert_compress_superset(s);
        }
      }
      // Get list
      size = 0;
      trie_invbfs.get_sets_list(list.data(), size, CK);
      trie_invbfs.clear();

      if (size > beam_size) {
        std::nth_element(list.begin(), list.begin() + beam_size, list.begin() + size, comparator);
        size = beam_size;
      }
    }

    return max_mlsw;
  }

  // not used
  uint64 get_automaton_lsw_cutoffinvbfs_tries(const Automaton<N, K>& aut,
      const InverseAutomaton<N, K>& invaut, const uint beam_size,
      const uint64 max_mlsw) {
    std::array<SetsTrie<N>, N> tries_invbfs;
    FastVector<Subset<N>> list(std::max(N, beam_size));
    uint size = 0;

    FastVector<bool> sink;
    get_automaton_reachable_states(
        aut, get_automaton_sink_component_vertex(aut), sink);
    for (uint n = 0; n < N; n++) {
      if (!sink[n]) continue;

      for (uint k = 0; k < K; k++) {
        if (invaut.end(n, k) - invaut.begin(n, k) > 1) {
          list[size++] = Subset<N>::Singleton(n);
          break;
        }
      }
    }

    for (uint64 len = 1; len < max_mlsw; len++) {
      // IBFS step
      for (uint i = 0; i < size; i++) {
        for (uint k = 0; k < K; k++) {
          Subset<N> s;
          list[i].invapply(invaut, k, s);
          uint subset_size = s.size();
          if (subset_size <= 1) continue;
          if (subset_size == N) {
            return len;
          }
          tries_invbfs[subset_size].insert_compress_superset(s);
        }
      }

      // Get list
      size = 0;
      for (int i = N - 1; i >= 0; i--) {
        tries_invbfs[i].get_sets_list(list.data(), size, beam_size);
        tries_invbfs[i].free();
      }
    }

    return max_mlsw;
  }

  /* Identity permutation.
   */
  std::array<int, N> get_identity_permutation() {
    std::array<int, N> arr;
    std::iota(arr.begin(), arr.end(), 0);
    return arr;
  }

  /* Sorted from lowest to highest in-degree.
   */
  std::array<int, N> get_indeg_permutation(const Automaton<N, K>& aut) {
    std::array<std::pair<double, int>, N> indeg;
    for (uint n = 0; n < N; ++n) {
      indeg[n] = {0, n};
    }

    for (uint n = 0; n < N; n++) {
      for (uint k = 0; k < K; k++) {
        indeg[aut[n][k]].first++;
      }
    }
    std::sort(indeg.begin(), indeg.end());

    std::array<int, N> order;
    for (uint n = 0; n < N; ++n) {
      order[indeg[n].second] = n;
    }
    return order;
  }
};

}  // namespace synchrolib

#ifndef __INTELLISENSE__
$BEAM_UNDEF$
#endif
