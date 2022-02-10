#pragma once
#include <synchrolib/data_structures/automaton.hpp>
#include <synchrolib/data_structures/subset.hpp>
#include <synchrolib/utils/general.hpp>
#include <synchrolib/utils/vector.hpp>
#include <optional>
#include <limits>

namespace synchrolib {

struct ReduceData {
  VarAutomaton aut;
  FastVector<FastVector<bool>> list_bfs;
  uint bfs_steps;

  bool done;  // set after reduction is done
};

struct AlgoResult {
  bool non_synchro;
  uint64 mlsw_lower_bound;
  uint64 mlsw_upper_bound;
  std::optional<FastVector<uint>> word;

  size_t algorithms_run;

  std::optional<ReduceData> reduce;  // used in Reduce, Exact

  AlgoResult():
      non_synchro(false),
      mlsw_lower_bound(0),
      mlsw_upper_bound(std::numeric_limits<uint64>::max()),
      algorithms_run(0) {}
};

template <uint N, uint K>
struct AlgoData {
  const Automaton<N, K> aut;
  const InverseAutomaton<N, K> invaut;

  AlgoResult result;

  AlgoData() {}
  AlgoData(Automaton<N, K> automaton):
      aut(automaton), invaut(aut) {}
  AlgoData(Automaton<N, K> automaton, InverseAutomaton<N, K> inverse_automaton):
      aut(automaton), invaut(inverse_automaton) {}
};

template <uint N, uint K>
class Algorithm {
public:
  virtual void run(AlgoData<N, K>& data) = 0;
  virtual ~Algorithm() = default;
};

}  // namespace synchrolib