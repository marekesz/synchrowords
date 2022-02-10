#include "preprocessed_transition_kernel.hpp"

#include <external/cuda/helper_cuda.h>
#include <cuda.h>

#include <synchrolib/data_structures/subset.hpp>
#include <synchrolib/utils/general.hpp>

#include <chrono>
#include <iostream>

#define BLK 262144

namespace {

template<synchrolib::uint N>
__global__ void count_bits_kernel_impl(const synchrolib::Subset<N>* subsets, int* acc, size_t cnt) {
  int start = blockIdx.x * BLK;
  int end = min(static_cast<size_t>((blockIdx.x + 1) * BLK), cnt);
  int b = threadIdx.x;

  int bucket = b / synchrolib::SUBSETS_BITS;
  int mask = (unsigned long long)1 << (b % synchrolib::SUBSETS_BITS);
  int sum = 0;

  for (int i = start; i < end; ++i) {
    if (subsets[i].v[bucket] & mask) {
      ++sum;
    }
  }

  atomicAdd(acc + b, sum);
}

} // namespace

namespace synchrolib {


template<uint N>
void count_bits_kernel(const Subset<N>* subsets, int* acc, size_t cnt) {
  const Subset<N>* subsets_ptr;
  int* acc_ptr;

std::chrono::steady_clock::time_point begin, end;
begin = std::chrono::steady_clock::now();
  checkCudaErrors(cudaMalloc((void **) &subsets_ptr, sizeof(Subset<N>) * cnt));
  checkCudaErrors(cudaMalloc((void **) &acc_ptr, sizeof(int) * N));

  checkCudaErrors(cudaMemcpy((void*) subsets_ptr, (void*) subsets, sizeof(Subset<N>) * cnt, cudaMemcpyHostToDevice));
  checkCudaErrors(cudaMemset(acc_ptr, 0, sizeof(int) * N));
end = std::chrono::steady_clock::now();
std::cout << "Copy time = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;

begin = std::chrono::steady_clock::now();
  count_bits_kernel_impl<N><<<(cnt + BLK - 1) / BLK, N/*, 0, stream*/>>>(
    subsets_ptr, acc_ptr, cnt);
  getLastCudaError("count_bits_kernel run failed\n"); // TODO: better error message

  checkCudaErrors(cudaMemcpy(acc, acc_ptr, sizeof(int) * N, cudaMemcpyDeviceToHost));
end = std::chrono::steady_clock::now();
std::cout << "Gpu time = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;

  checkCudaErrors(cudaFree((void*) subsets_ptr));
  checkCudaErrors(cudaFree((void*) acc_ptr));
}

template void count_bits_kernel<AUT_N>(const Subset<AUT_N>* subsets, int* acc, size_t cnt);

}  // namespace synchrolib

#undef BLK
