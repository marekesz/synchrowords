#include "connectivity.hpp"

// TODO: refactor

namespace synchrolib {

template <uint N, uint K>
uint CheckerSC<N, K>::dfs(const uint v) {
  uint low = indices[v] = index++;
  for (uint k = 0; k < K; k++) {
    uint v2 = aut[v][k];
    if (indices[v2]) {
      if (indices[v2] < low) low = indices[v2];
    } else {
      uint low2 = dfs(v2);
      if (!low2) return 0;
      if (low2 < low) low = low2;
    }
  }
  if (indices[v] == low) return 0;
  return low;
}

template<uint N, uint K>
CheckerSC<N, K>::CheckerSC(const Automaton<N, K> &a) : aut(a) {}

template<uint N, uint K>
bool CheckerSC<N, K>::check() {
  for (uint n = 1; n < N; n++) indices[n] = 0;
  indices[0] = 1;
  index = 2;
  for (uint k = 0; k < K; k++)
    if (!indices[aut[0][k]])
      if (!dfs(aut[0][k])) return false;
  for (uint n = 1; n < N; n++)
    if (!indices[n]) return false;
  return true;
}

template <uint N, uint K>
bool is_automaton_strongly_connected(const Automaton<N, K> &aut) {
  return CheckerSC<N, K>(aut).check();
}

template <uint N, uint K>
void CheckerComponents<N, K>::strongconnect(const uint v) {
  // Set the depth index for v to the smallest unused index
  indices[v] = lowlink[v] = index;
  index++;
  stack[stacktop++] = v;
  onStack[v] = true;

  // Consider successors of v
  for (uint k = 0; k < K; k++) {
    if (!indices[aut[v][k]]) {
      // Successor w has not yet been visited; recurse on it
      strongconnect(aut[v][k]);
      if (lowlink[aut[v][k]] < lowlink[v]) lowlink[v] = lowlink[aut[v][k]];
    } else if (onStack[aut[v][k]]) {
      // Successor w is in stack S and hence in the current SCC
      if (indices[aut[v][k]] < lowlink[v]) lowlink[v] = indices[aut[v][k]];
    }
  }

  // If v is a root node, pop the stack and generate an SCC
  if (lowlink[v] == indices[v]) {
    uint w;
    do {
      w = stack[--stacktop];
      onStack[w] = false;
      component[w] = numComponents;
    } while (w != v);
    numComponents++;
  }
}

template<uint N, uint K>
CheckerComponents<N, K>::CheckerComponents(const Automaton<N, K> &a) : aut(a) {
  numComponents = 0;
  for (uint n = 0; n < N; n++) {
    indices[n] = 0;
    onStack[n] = false;
  }
  index = 1;
  stacktop = 0;
  for (uint v = 0; v < N; v++)
    if (!indices[v]) strongconnect(v);
}

template <uint N, uint K>
void get_automaton_strongly_connected_components(
    const Automaton<N, K> &aut, uint component[]) {
  CheckerComponents<N, K> checker(aut);
  for (uint n = 0; n < N; n++) component[n] = checker.component[n];
}

template <uint N, uint K>
void get_automaton_strongly_connected_components(
    const Automaton<N, K> &aut, uint component[], uint &num_components) {
  CheckerComponents<N, K> checker(aut);
  for (uint n = 0; n < N; n++) component[n] = checker.component[n];
  num_components = checker.numComponents;
}

template<uint N, uint K>
uint CheckerSinkComponent<N, K>::dfs(const uint v) {
  uint low = indices[v] = index++;
  for (uint k = 0; k < K; k++) {
    uint v2 = aut[v][k];
    if (indices[v2]) {
      if (indices[v2] < low) low = indices[v2];
    } else {
      uint low2 = dfs(v2);
      if (!low2) return 0;
      if (low2 < low) low = low2;
    }
  }
  if (indices[v] == low) {
    sink = v;
    return 0;
  }
  return low;
}

template<uint N, uint K>
CheckerSinkComponent<N, K>::CheckerSinkComponent(const Automaton<N, K> &a) : aut(a) {}

template<uint N, uint K>
uint CheckerSinkComponent<N, K>::check() {
  for (uint n = 0; n < N; n++) indices[n] = 0;
  sink = 0;
  index = 1;
  dfs(0);
  return sink;
}

template <uint N, uint K>
uint get_automaton_sink_component_vertex(const Automaton<N, K> &aut) {
  return CheckerSinkComponent<N, K>(aut).check();
}

template <uint N, uint K>
void get_automaton_reachable_states(
    const Automaton<N, K> &aut, const uint v, FastVector<bool> &reachable) {
  reachable.resize(N);
  std::fill(reachable.begin(), reachable.end(), false);
  reachable[v] = true;
  uint stack[N], top = 1;
  stack[0] = v;
  while (top > 0) {
    uint n = stack[--top];
    for (uint k = 0; k < K; k++) {
      uint p = aut[n][k];
      if (!reachable[p]) {
        reachable[p] = true;
        stack[top++] = p;
      }
    }
  }
}

template class CheckerSC<AUT_N, AUT_K>;
template bool is_automaton_strongly_connected(const Automaton<AUT_N, AUT_K> &aut);
template class CheckerComponents<AUT_N, AUT_K>;
template class CheckerSinkComponent<AUT_N, AUT_K>;

template void get_automaton_strongly_connected_components(
    const Automaton<AUT_N, AUT_K> &aut, uint component[]);

template void get_automaton_strongly_connected_components(
    const Automaton<AUT_N, AUT_K> &aut, uint component[], uint &num_components);

template uint get_automaton_sink_component_vertex(const Automaton<AUT_N, AUT_K> &aut);

template void get_automaton_reachable_states(
    const Automaton<AUT_N, AUT_K> &aut, const uint v, FastVector<bool> &reachable);

}  // namespace synchrolib