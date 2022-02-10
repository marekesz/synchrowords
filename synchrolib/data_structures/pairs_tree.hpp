#pragma once
#include <synchrolib/data_structures/automaton.hpp>
#include <synchrolib/data_structures/subset.hpp>
#include <synchrolib/utils/general.hpp>
#include <synchrolib/utils/vector.hpp>
#include <synchrolib/utils/timer.hpp>
#include <cstring>
#include <utility>
#include <vector>

namespace synchrolib {

template <uint N, uint K>
class PairsTree : public NonCopyable, public NonMovable {  // TODO: make movable
protected:
  struct pair_t {
    uint first, second;
  };
  struct info_t {
    uint length, letter;
  };
  info_t *info;
  pair_t *queue;  // deleted after construction

  uint calculate_tree(const Automaton<N, K> &aut) {
    Timer timer_bfs("calculate_tree/init");
    auto inv = InverseAutomaton<N, K>(aut);
    uint top = 0, bottom = 0;
    for (uint n = 0; n < N; n++) {
      for (uint k = 0; k < K; k++) {
        for (uint i1 = inv.begin(n, k); i1 < inv.end(n, k); i1++) {
          for (uint i2 = i1 + 1; i2 < inv.end(n, k); i2++) {
            uint n1 = inv.edge(i1, k);
            uint n2 = inv.edge(i2, k);
            if (info[n1 * N + n2].length) continue;
            info[n1 * N + n2] = info[n2 * N + n1] = {1, k};
            queue[top] = {n1, n2};
            top++;
          }
        }
      }
    }
    timer_bfs.stop();

    Timer timer_bfs2("calculate_tree/bfs");
    while (top < N * (N - 1) / 2 && top > bottom) {
      auto &[v1, v2] = queue[bottom];
      uint l = info[v1 * N + v2].length + 1;
      bottom++;
      for (uint k = 0; k < K; k++) {
        for (uint i1 = inv.begin(v1, k); i1 < inv.end(v1, k); i1++) {
          for (uint i2 = inv.begin(v2, k); i2 < inv.end(v2, k); i2++) {
            uint n1 = inv.edge(i1, k);
            uint n2 = inv.edge(i2, k);
            if (info[n1 * N + n2].length) continue;
            info[n1 * N + n2] = info[n2 * N + n1] = {l, k};
            queue[top] = {n1, n2};
            top++;
          }
        }
      }
    }
    timer_bfs2.stop();
    return top;
  }

public:
  void initialize(const Automaton<N, K> &aut) {
    Timer timer_constructor("PairsTree initialize");
    info = new info_t[N * N]();
    queue = new pair_t[N * (N - 1) / 2];
    uint top = calculate_tree(aut);
    delete[] queue;
  }

  ~PairsTree() {
    delete[] info;
  }

  __attribute__((always_inline)) inline uint get_letter(
      const uint n1, const uint n2) const {
    return info[n1 * N + n2].letter;
  }
  __attribute__((always_inline)) inline uint get_length(
      const uint n1, const uint n2) const {
    return info[n1 * N + n2].length;
  }

  uint get_max_length() const {
    uint len = 0;
    for (uint i = 0; i < N * N; ++i) {
      if (info[i].length > len) {
        len = info[i].length;
      }
    }
    return len;
  }

  bool is_synchronizing() const {
    for (uint n1 = 0; n1 < N - 1; n1++)
      for (uint n2 = n1 + 1; n2 < N; n2++)
        if (!info[n1 * N + n2].length) return false;
    return true;
  }

  // O(nl)
  void apply(
      const Automaton<N, K> &aut, uint &n1, uint &n2, Subset<N> &subset) const {
    while (n1 != n2) {
      const uint k = info[n1 * N + n2].letter;
      Subset<N> s;
      subset.apply(aut, k, s);
      subset = s;
      n1 = aut[n1][k];
      n2 = aut[n2][k];
    }
  }

  // O(nl)
  void apply(const Automaton<N, K> &aut, uint &n1, uint &n2, Subset<N> &subset,
      FastVector<uint> &word) const {
    while (n1 != n2) {
      const uint k = info[n1 * N + n2].letter;
      word.push_back(k);
      Subset<N> s;
      subset.apply(aut, k, s);
      subset = s;
      n1 = aut[n1][k];
      n2 = aut[n2][k];
    }
  }
};

template <uint N, uint K>
class PairsTreeTransitionTables : public PairsTree<N, K> {
private:
  using PairsTree<N, K>::info;
  using PairsTree<N, K>::queue;
  using typename PairsTree<N, K>::info_t;
  using typename PairsTree<N, K>::pair_t;
  using PairsTree<N, K>::get_max_length;

  uint **transition;
  uint *transition_data;

  void calculate_transitions(const Automaton<N, K> &aut, uint top) {
    Timer timer_transition1("calculate_transition/get_max_length");
    if (get_max_length() < N) {
      return;
    }
    timer_transition1.stop();

    Timer timer_transition2("calculate_transition/depth");
    uint *depth = new uint[N * N]();
    uint *fre = transition_data;
    for (int i = top - 1; i >= 0; i--) {
      auto &[v1, v2] = queue[i];
      uint d = depth[v1 * N + v2] + 1;
      if (d >= N) {
        transition[v1 * N + v2] = transition[v2 * N + v1] = fre;
        fre += N;
      } else {
        uint n1 = aut[v1][info[v1 * N + v2].letter];
        uint n2 = aut[v2][info[v1 * N + v2].letter];
        if (d > depth[n1 * N + n2]) {
          depth[n1 * N + n2] = depth[n2 * N + n1] = d;
        }
      }
    }
    delete[] depth;
    timer_transition2.stop();

    Timer timer_transition3("calculate_transition/calc");
    for (uint i = 0; i < top; i++) {
      auto &[v1, v2] = queue[i];
      if (transition[v1 * N + v2]) {
        for (uint n = 0; n < N; n++) {
          uint n1 = v1;
          uint n2 = v2;
          uint nx = n;
          do {
            uint k = info[n1 * N + n2].letter;
            n1 = aut[n1][k];
            n2 = aut[n2][k];
            nx = aut[nx][k];
            if (transition[n1 * N + n2]) {
              nx = transition[n1 * N + n2][nx];
              break;
            }
          } while (n1 != n2);
          transition[v1 * N + v2][n] = nx;
        }
      }
    }
    timer_transition3.stop();
  }

public:
  using PairsTree<N, K>::calculate_tree;

  void initialize(const Automaton<N, K> &aut) {
    Timer timer_constructor("PairsTreeTransitionTables initialize");
    info = new info_t[N * N]();
    queue = new pair_t[N * (N - 1) / 2];
    uint top = calculate_tree(aut);
    transition = new uint *[N * N]();
    transition_data = new uint[N * N];
    calculate_transitions(aut, top);
    delete[] queue;
  }

  ~PairsTreeTransitionTables() {
    delete[] transition_data;
    delete[] transition;
  }

  // O(min(n^2, nl))
  void apply(
      const Automaton<N, K> &aut, uint &n1, uint &n2, Subset<N> &subset) const {
    while (n1 != n2) {
      if (transition[n1 * N + n2]) {
        Subset<N> s = Subset<N>::Empty();
        for (uint n = 0; n < N; n++)
          if (subset.is_set(n)) s.set(transition[n1 * N + n2][n]);
        subset = s;
        n1 = n2 = transition[n1 * N + n2][n1];
        break;
      }
      const uint k = info[n1 * N + n2].letter;
      Subset<N> s;
      subset.apply(aut, k, s);
      subset = s;
      n1 = aut[n1][k];
      n2 = aut[n2][k];
    }
  }
};

}  // namespace synchrolib
