#include <jitdefines.hpp>

#include <synchrolib/synchrolib.hpp>
#include <iostream>
#include <memory>

extern "C" {

using namespace synchrolib;

void run(const std::string& aut_encoded,
    const std::vector<std::string>& algorithms, AlgoResult& result,
    Logger::LogLevel log_level) {
  Timer timer("algorithms");

  Logger::set_log_level(log_level);

  AlgoData<AUT_N, AUT_K> data = AlgoData<AUT_N, AUT_K>(
      Automaton<AUT_N, AUT_K>::decode(aut_encoded));

  if (result.reduce && result.reduce->done) {
    data.result = result;
  } else {
    data.result.non_synchro = false;
    data.result.mlsw_lower_bound = 0;
    data.result.algorithms_run = 0;

    using BoundType = decltype(data.result.mlsw_upper_bound);
    data.result.mlsw_upper_bound = std::numeric_limits<BoundType>::max();
    if (UPPER_BOUND < data.result.mlsw_upper_bound) {
      data.result.mlsw_upper_bound = static_cast<BoundType>(UPPER_BOUND);
    }
  }

  while (data.result.algorithms_run < algorithms.size()) {
    Timer timer("algo" + algorithms[data.result.algorithms_run]);
    make_algorithm<AUT_N, AUT_K>(algorithms[data.result.algorithms_run])
        ->run(data);
    data.result.algorithms_run++;

    if (data.result.reduce && !data.result.reduce->done) {
      result = data.result;
      return;
    }
  }

  result = data.result;
}
}
