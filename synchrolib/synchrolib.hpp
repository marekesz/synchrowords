#pragma once
#include <synchrolib/utils/general.hpp>
#include <synchrolib/utils/logger.hpp>
#include <synchrolib/utils/timer.hpp>
#include <synchrolib/algorithm/algorithm.hpp>

#ifdef JIT

// #ifdef COMPILE_ALGORITHMNAME
// #include <synchrolib/algorithm/algorithmname/algorithmname.hpp>
// #endif

#ifdef COMPILE_EPPSTEIN
#include <synchrolib/algorithm/eppstein/eppstein.hpp>
#endif

#ifdef COMPILE_BEAM
#include <synchrolib/algorithm/beam/beam.hpp>
#endif

#ifdef COMPILE_EXACT
#include <synchrolib/algorithm/exact/exact.hpp>
#endif

#ifdef COMPILE_BRUTE
#include <synchrolib/algorithm/brute/brute.hpp>
#endif

#ifdef COMPILE_REDUCE
#include <synchrolib/algorithm/reduce/reduce.hpp>
#endif

#else

#include <synchrolib/algorithm/config.hpp>

// #include <synchrolib/algorithm/algorithmname/config.hpp>
#include <synchrolib/algorithm/beam/config.hpp>
#include <synchrolib/algorithm/brute/config.hpp>
#include <synchrolib/algorithm/eppstein/config.hpp>
#include <synchrolib/algorithm/exact/config.hpp>
#include <synchrolib/algorithm/reduce/config.hpp>

#endif

#include <memory>
#include <stdexcept>

namespace synchrolib {

#ifdef JIT

template <uint N, uint K>
std::unique_ptr<Algorithm<N, K>> make_algorithm(std::string name) {
  // #ifdef COMPILE_ALGORITHMNAME
  //   if (name == "AlgorithmName") {
  //       return std::make_unique<AlgorithmName<N, K>>();
  //   }
  // #endif

#ifdef COMPILE_EPPSTEIN
  if (name == "Eppstein") {
    return std::make_unique<Eppstein<N, K>>();
  }
#endif

#ifdef COMPILE_BEAM
  if (name == "Beam") {
    return std::make_unique<Beam<N, K>>();
  }
#endif

#ifdef COMPILE_EXACT
  if (name == "Exact") {
    return std::make_unique<Exact<N, K>>();
  }
#endif

#ifdef COMPILE_BRUTE
  if (name == "Brute") {
    return std::make_unique<Brute<N, K>>();
  }
#endif

#ifdef COMPILE_REDUCE
  if (name == "Reduce") {
    return std::make_unique<Reduce<N, K>>();
  }
#endif

  throw std::runtime_error("Unknown algorithm: " + name);
}

#else

std::unique_ptr<AlgoConfig> make_algo_config(std::string name) {
  //   if (name == "AlgorithmName") {
  //     return std::make_unique<AlgorithmNameConfig>();
  //   }

  if (name == "Eppstein") {
    return std::make_unique<EppsteinConfig>();
  }

  if (name == "Beam") {
    return std::make_unique<BeamConfig>();
  }

  if (name == "Exact") {
    return std::make_unique<ExactConfig>();
  }

  if (name == "Brute") {
    return std::make_unique<BruteConfig>();
  }

  if (name == "Reduce") {
    return std::make_unique<ReduceConfig>();
  }

  throw std::runtime_error("Unknown algorithm: " + name);
}

#endif

}  // namespace synchrolib