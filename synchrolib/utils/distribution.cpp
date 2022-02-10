#include "distribution.hpp"

namespace synchrolib {

MatrixSqr::MatrixSqr(const uint _N) : N(_N) { data = new double[N * N]; }

MatrixSqr::~MatrixSqr() { delete[] data; }

void MatrixSqr::invert() {
  // Matrix inversion by Gaussian Elimination with LU decomposition, based on
  // Mike Dinolfo 12/98 code.
  // -- the matrix must be invertible
  // -- the matrix must have non-zero values in the diagonal
  // -- dimensions must be at least 2
  for (uint i = 1; i < N; i++) data[i] /= data[0];  // Normalize row 0
  for (uint i = 1; i < N; i++) {
    for (uint j = i; j < N; j++) {  // Do a column of L
      double sum = 0.0;
      for (uint k = 0; k < i; k++)
        sum += data[j * N + k] * data[k * N + i];
      data[j * N + i] -= sum;
    }
    if (i == N - 1) continue;
    for (uint j = i + 1; j < N; j++) {  // Do a row of U
      double sum = 0.0;
      for (uint k = 0; k < i; k++)
        sum += data[i * N + k] * data[k * N + j];
      data[i * N + j] = (data[i * N + j] - sum) / data[i * N + i];
    }
  }
  for (uint i = 0; i < N; i++)  // Invert L
    for (uint j = i; j < N; j++) {
      double x = 1.0;
      if (i != j) {
        x = 0.0;
        for (uint k = i; k < j; k++)
          x -= data[j * N + k] * data[k * N + i];
      }
      data[j * N + i] = x / data[j * N + j];
    }
  for (uint i = 0; i < N; i++)  // Invert U
    for (uint j = i; j < N; j++) {
      if (i == j) continue;
      double sum = 0.0;
      for (uint k = i; k < j; k++)
        sum += data[k * N + j] * ((i == k) ? 1.0 : data[i * N + k]);
      data[i * N + j] = -sum;
    }
  for (uint i = 0; i < N; i++)  // Final inversion
    for (uint j = 0; j < N; j++) {
      double sum = 0.0;
      for (uint k = ((i > j) ? i : j); k < N; k++)
        sum += ((j == k) ? 1.0 : data[j * N + k]) * data[k * N + i];
      data[j * N + i] = sum;
    }
};

void get_stationary_distribution(const uint n,
    const FastVector<double>& prob, FastVector<double>& pi) {
  // \pi = u[I-P+U]^{-1}
  MatrixSqr mat(n);
  for (uint n1 = 0; n1 < n; n1++) {
    for (uint n2 = 0; n2 < n; n2++) {
      mat[n1][n2] = (double)1.0 / n - prob[n1 * n + n2];
    }
    mat[n1][n1] += 1.0;
  }
  mat.invert();
  for (uint n1 = 0; n1 < n; n1++) {
    pi[n1] = 0.0;
    for (uint n2 = 0; n2 < n; n2++) pi[n1] += mat[n2][n1];
    pi[n1] /= n;
  }
}

void get_automaton_stationary_distribution(
    const VarAutomaton& aut, FastVector<double>& pi) {
  FastVector<double> prob(aut.N * aut.N);
  for (uint n = 0; n < aut.N; n++) {
    double w = 1.0 / (aut.N * aut.N * aut.K * (1.0 / (aut.N * aut.K) + 1.0));
    for (uint m = 0; m < aut.N; m++) prob[n * aut.N + m] = w;
    w = 1.0 / (aut.K * (1.0 / (aut.N * aut.K) + 1.0));
    for (uint k = 0; k < aut.K; k++) {
      prob[n * aut.N + aut[n][k]] += w;
    }
  }
  get_stationary_distribution(aut.N, prob, pi);
}

}  // namespace synchrolib
