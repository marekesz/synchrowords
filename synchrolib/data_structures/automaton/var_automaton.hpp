#pragma once
#include <algorithm>
#include <array>
#include <synchrolib/utils/general.hpp>
#include <cassert>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace synchrolib {

// TODO: copy, move ctors
struct VarAutomaton {
  uint N, K;
  uint* t;

  VarAutomaton() : N(0), K(0), t(NULL) {}

  VarAutomaton(const uint n, const uint k) : N(n), K(k), t(new uint[N * K]) {}

  VarAutomaton(const VarAutomaton& aut)
      : N(aut.N), K(aut.K), t(new uint[N * K]) {
    for (uint i = 0; i < N * K; i++) t[i] = aut.t[i];
  }

  VarAutomaton(const uint n, const uint k, const std::string& c)
      : N(n), K(k), t(new uint[N * K]) {
    std::istringstream is(c, std::istringstream::in);
    for (uint i = 0; i < K * N; i++) is >> t[i];
  }

  VarAutomaton(const uint n, const uint k, std::istream& is)
      : N(n), K(k), t(new uint[N * K]) {
    for (uint i = 0; i < K * N; i++) is >> t[i];
  }

  template <uint _N, uint _K>
  VarAutomaton(const Automaton<_N, _K>& aut)
      : N(_N), K(_K), t(new uint[N * K]) {
    for (uint n = 0; n < N; ++n) {
      for (uint k = 0; k < K; ++k) {
        (*this)[n][k] = aut[n][k];
      }
    }
  }

  ~VarAutomaton() { delete[] t; }

  VarAutomaton& operator=(const VarAutomaton& aut) {
    delete[] t;
    K = aut.K;
    N = aut.N;
    t = new uint[N * K];
    for (uint i = 0; i < K * N; i++) t[i] = aut.t[i];
    return *this;
  }

  uint* operator[](const uint n) { return t + (K * n); }
  const uint* operator[](const uint n) const { return t + (K * n); }

  bool operator<(const VarAutomaton& a) const {
    for (uint i = 0; i < K * N - 1; i++)
      if (t[i] != a.t[i]) return t[i] < a.t[i];
    return (t[K * N - 1] < a.t[K * N - 1]);
  }

  __attribute__((always_inline)) inline bool operator>(
      const VarAutomaton& a) const {
    return (a < (*this));
  }

  bool operator==(const VarAutomaton& a) const {
    for (uint i = 0; i < K * N; i++)
      if (t[i] != a.t[i]) return false;
    return true;
  }
  __attribute__((always_inline)) inline bool operator!=(
      const VarAutomaton& a) const {
    return !((*this) == a);
  }

  std::string code() const {
    std::ostringstream oss(std::ostringstream::out);
    oss << t[0];
    for (uint i = 1; i < K * N; i++) oss << " " << t[i];
    return oss.str();
  }

  std::string codek() const {
    std::ostringstream oss(std::ostringstream::out);
    for (uint k = 0; k < K; k++) {
      oss << "[" << (*this)[0][k];
      for (uint n = 1; n < N; n++) oss << "," << (*this)[n][k];
      oss << "]";
    }
    return oss.str();
  }
};

}  // namespace synchrolib
