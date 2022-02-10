#include "preprocessed_transition_kernel.hpp"

#include <external/cuda/helper_cuda.h>
#include <cuda.h>

#include <synchrolib/data_structures/preprocessed_transition.hpp>
#include <synchrolib/data_structures/subset.hpp>
#include <synchrolib/utils/bits.hpp>
#include <synchrolib/utils/general.hpp>

#include <chrono>
#include <iostream>

#define BLK 256

namespace {

template<synchrolib::uint N, synchrolib::uint K, synchrolib::uint B, synchrolib::uint S>
__global__ void preprocessed_transition_kernel_impl(
    const synchrolib::PreprocessedTransition<N, K>* trans,
    const synchrolib::Subset<N>* from,
    synchrolib::Subset<N>* to,
    size_t cnt) {

  int ind = blockIdx.x * BLK + threadIdx.x;
  if (ind >= cnt) {
    return;
  }

  synchrolib::Subset<N> sub = from[ind];
  synchrolib::Subset<N> ret;
  for (synchrolib::uint i = 0; i < B; ++i) {
    ret.v[i] = 0;
  }

  for (synchrolib::uint i = 0; i < S; i++) {
    const synchrolib::uint b = i * synchrolib::PREPROCESSED_TRANSITION_SLICE / synchrolib::SUBSETS_BITS;
    const synchrolib::uint shift = i * synchrolib::PREPROCESSED_TRANSITION_SLICE % synchrolib::SUBSETS_BITS;
    const synchrolib::uint set = (synchrolib::uint)(
        (sub.v[b] >> (shift)) & ((1 << synchrolib::PREPROCESSED_TRANSITION_SLICE) - 1));
    for (synchrolib::uint b = 0; b < B; ++b) {
      ret.v[b] |= trans->trans[i][set].v[b];
    }
  }

  to[ind] = ret;
}

} // namespace

namespace synchrolib {

template<uint N, uint K>
void preprocessed_transition_kernel(const PreprocessedTransition<N, K>& trans, const Subset<N>* from, Subset<N>* to, size_t cnt) {
  // for (size_t i = 0; i < cnt; ++i, ++from, ++to) {
  //   trans.apply(*from, *to);
  // }
  const PreprocessedTransition<N, K>* trans_ptr;
  const Subset<N>* from_ptr;
  Subset<N>* to_ptr;

// std::chrono::steady_clock::time_point begin, end;
// begin = std::chrono::steady_clock::now();
  checkCudaErrors(cudaMalloc((void **) &trans_ptr, sizeof(PreprocessedTransition<N, K>)));
  checkCudaErrors(cudaMalloc((void **) &from_ptr, sizeof(Subset<N>) * cnt));
  checkCudaErrors(cudaMalloc((void **) &to_ptr, sizeof(Subset<N>) * cnt));

  checkCudaErrors(cudaMemcpy((void*) trans_ptr, (void*) &trans, sizeof(PreprocessedTransition<N, K>), cudaMemcpyHostToDevice));
  checkCudaErrors(cudaMemcpy((void*) from_ptr, (void*) from, sizeof(Subset<N>) * cnt, cudaMemcpyHostToDevice));
  checkCudaErrors(cudaMemset(to_ptr, 0, sizeof(Subset<N>) * cnt));
// end = std::chrono::steady_clock::now();
// std::cout << "Copy time = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;


// begin = std::chrono::steady_clock::now();
  preprocessed_transition_kernel_impl<N, K, Subset<N>::buckets(), PreprocessedTransition<N, K>::slices()><<<(cnt + BLK - 1) / BLK, BLK/*, 0, stream*/>>>(
    trans_ptr, from_ptr, to_ptr, cnt);
  getLastCudaError("preprocessed_transition_kernel run failed\n"); // TODO: better error message

  checkCudaErrors(cudaMemcpy(to, to_ptr, sizeof(Subset<N>) * cnt, cudaMemcpyDeviceToHost));
// end = std::chrono::steady_clock::now();
// std::cout << "Gpu time = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;

  checkCudaErrors(cudaFree((void*) trans_ptr));
  checkCudaErrors(cudaFree((void*) from_ptr));
  checkCudaErrors(cudaFree(to_ptr));
}

template void preprocessed_transition_kernel<AUT_N, AUT_K>(const PreprocessedTransition<AUT_N, AUT_K>& trans, const Subset<AUT_N>* from, Subset<AUT_N>* to, size_t cnt);

}  // namespace synchrolib

#undef BLK
