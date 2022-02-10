#pragma once
#include <jitdefines.hpp>

#include <synchrolib/data_structures/automaton.hpp>
#include <synchrolib/utils/general.hpp>
#include <vector>

namespace synchrolib {

// ********************************************************************************************************************
// Strong connectivity checker

/**
 * Strong connectivity checker,
 * uses simplified version of Tarjan's algorithm.
 */
template <uint N, uint K>
class CheckerSC {
private:
  const Automaton<N, K> aut;
  // uint index,indices[N];
  uint indices[N], index;
  uint dfs(const uint v);

public:
  CheckerSC(const Automaton<N, K> &a);
  bool check();
};

/**
 * Checks if the automaton is strongly connected.
 */
template <uint N, uint K>
bool is_automaton_strongly_connected(const Automaton<N, K> &aut);

// ********************************************************************************************************************
// Components checker

/**
 * Components checker by Tarjan's algorithm.
 */
template <uint N, uint K>
class CheckerComponents {
  const Automaton<N, K> aut;
  uint lowlink[N];
  uint indices[N], index;
  uint stack[N], stacktop;
  bool onStack[N];

  void strongconnect(const uint v);
public:
  uint component[N], numComponents;
  CheckerComponents(const Automaton<N, K> &a);
};

template <uint N, uint K>
void get_automaton_strongly_connected_components(
    const Automaton<N, K> &aut, uint component[]);

template <uint N, uint K>
void get_automaton_strongly_connected_components(
    const Automaton<N, K> &aut, uint component[], uint &num_components);

// ********************************************************************************************************************
// Sink component

/**
 * Checker for a sink component,
 * uses a simplified version of Tarjan's algorithm.
 */
template <uint N, uint K>
class CheckerSinkComponent {
  const Automaton<N, K> aut;
  uint sink, index, indices[N];
  uint dfs(const uint v);

public:
  CheckerSinkComponent(const Automaton<N, K> &a);
  uint check();
};

/**
 * Gets a vertex of the sink component, the automaton must have a sink.
 */
template <uint N, uint K>
uint get_automaton_sink_component_vertex(const Automaton<N, K> &aut);

template <uint N, uint K>
void get_automaton_reachable_states(
    const Automaton<N, K> &aut, const uint v, FastVector<bool> &reachable);

}  // namespace synchrolib