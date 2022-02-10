#pragma once
#include <synchrolib/data_structures/automaton.hpp>
#include <synchrolib/utils/general.hpp>

namespace synchrolib {

// TODO: copy, move ctors
class MatrixSqr : public NonMovable, NonCopyable {
private:
  const uint N;
  double* data;

public:
  MatrixSqr(const uint _N);
  ~MatrixSqr();
  inline const double* operator[](const uint n) const {
    return data + n * N;
  }
  inline double* operator[](const uint n) { return data + n * N; }

  void invert();
};

void get_stationary_distribution(const uint n,
    const FastVector<double>& prob, FastVector<double>& pi);

void get_automaton_stationary_distribution(
    const VarAutomaton& aut, FastVector<double>& pi);

}  // namespace synchrolib
