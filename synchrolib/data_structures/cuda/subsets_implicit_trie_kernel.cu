#include <synchrolib/data_structures/cuda/subsets_implicit_trie_kernel.hpp>

#include <external/cuda/helper_cuda.h>
#include <cuda.h>

#include <iostream>

#define BLK 256

namespace {

template<synchrolib::uint N, synchrolib::uint B>
__global__ void subsets_trie_kernel_impl_v2(
    synchrolib::Subset<N>* small,
    synchrolib::Subset<N>* large,
    int small_cnt,
    int large_cnt,
    bool* results) {
  int id = blockIdx.x * BLK + threadIdx.x;
  if (id >= large_cnt) {
    return;
  }

  bool ret = false;
  for (int i = 0; i < small_cnt; ++i) {
    synchrolib::Subset<N> they = small[i];

    bool good = true;
    for (int j = 0; j < B; ++j) {
      if ((large[id].v[j] & they.v[j]) != they.v[j]) {
        good = false;
      }
    }

    if (good) {
      ret = true;
    }
  }

  // if (__any_sync(0xffffffff, ret)) {
  //   if (t == 0) {
  //     results[blockIdx.x] = true;
  //   }
  // }
  if (ret) {
    results[id] = true; // TODO: test speed
  }
}

template<synchrolib::uint N, synchrolib::uint B>
__global__ void subsets_trie_proper_kernel_impl_v2(
    synchrolib::Subset<N>* small,
    synchrolib::Subset<N>* large,
    int small_cnt,
    int large_cnt,
    bool* results) {
  int id = blockIdx.x * BLK + threadIdx.x;
  if (id >= large_cnt) {
    return;
  }

  bool ret = false;
  for (int i = 0; i < small_cnt; ++i) {
    synchrolib::Subset<N> they = small[i];

    bool good = true;
    bool same = true;
    for (int j = 0; j < B; ++j) {
      if ((large[id].v[j] & they.v[j]) != they.v[j]) {
        good = false;
      }

      if (large[id].v[j] != they.v[j]) {
        same = false;
      }
    }

    if (good && !same) {
      ret = true;
    }
  }

  // if (__any_sync(0xffffffff, ret)) {
  //   if (threadIdx.x == 0) {
  //     results[blockIdx.x] = true;
  //   }
  // }
  if (ret) {
    results[id] = true; // TODO: test speed
  }
}




template<synchrolib::uint N, synchrolib::uint B>
__global__ void subsets_trie_kernel_impl(
    synchrolib::Subset<N>* small,
    synchrolib::Subset<N>* large,
    int small_cnt,
    bool* results) {
  synchrolib::Subset<N> sub = large[blockIdx.x];

  bool ret = false;
  for (int i = threadIdx.x; i < small_cnt; i += blockDim.x) {
    synchrolib::Subset<N> they = small[i];

    bool good = true;
    for (int j = 0; j < B; ++j) {
      if ((sub.v[j] & they.v[j]) != they.v[j]) {
        good = false;
      }
    }

    if (good) {
      ret = true;
    }
  }

  // if (__any_sync(0xffffffff, ret)) {
  //   if (threadIdx.x == 0) {
  //     results[blockIdx.x] = true;
  //   }
  // }
  if (ret) {
    results[blockIdx.x] = true; // TODO: test speed
  }
}

template<synchrolib::uint N, synchrolib::uint B>
__global__ void subsets_trie_proper_kernel_impl(
    synchrolib::Subset<N>* small,
    synchrolib::Subset<N>* large,
    int small_cnt,
    bool* results) {
  synchrolib::Subset<N> sub = large[blockIdx.x];

  bool ret = false;
  for (int i = threadIdx.x; i < small_cnt; i += blockDim.x) {
    synchrolib::Subset<N> they = small[i];

    bool good = true;
    bool same = true;
    for (int j = 0; j < B; ++j) {
      if ((sub.v[j] & they.v[j]) != they.v[j]) {
        good = false;
      }

      if (sub.v[j] != they.v[j]) {
        same = false;
      }
    }

    if (good && !same) {
      ret = true;
    }
  }

  // if (__any_sync(0xffffffff, ret)) {
  //   if (threadIdx.x == 0) {
  //     results[blockIdx.x] = true;
  //   }
  // }
  if (ret) {
    results[blockIdx.x] = true; // TODO: test speed
  }
}

} // namespace


namespace synchrolib {

template<uint N, bool Proper>
SubsetsImplicitTrieKernel<N, Proper>::SubsetsImplicitTrieKernel(): allocated(false) {}

template<uint N, bool Proper>
SubsetsImplicitTrieKernel<N, Proper>::~SubsetsImplicitTrieKernel() {
  deallocate();
}

template<uint N, bool Proper>
void SubsetsImplicitTrieKernel<N, Proper>::run(
    uint small_pos,
    uint small_cnt,
    const Subset<N>* large,
    uint large_cnt,
    bool* ret) {
  checkCudaErrors(cudaMemcpy(large_ptr, large, sizeof(Subset<N>) * large_cnt, cudaMemcpyHostToDevice));
  checkCudaErrors(cudaMemset(results, 0, sizeof(bool) * large_cnt));

  if (Proper) {
    subsets_trie_proper_kernel_impl_v2<N, Subset<N>::buckets()><<<(large_cnt + BLK - 1) / BLK, BLK/*, 0, stream*/>>>(
      small_ptr + small_pos, large_ptr, small_cnt, large_cnt, results);
  } else {
    subsets_trie_kernel_impl_v2<N, Subset<N>::buckets()><<<(large_cnt + BLK - 1) / BLK, BLK/*, 0, stream*/>>>(
      small_ptr + small_pos, large_ptr, small_cnt, large_cnt, results);
  }
  // if (Proper) {
  //   subsets_trie_proper_kernel_impl<N, Subset<N>::buckets()><<<large_cnt, 32/*, 0, stream*/>>>(
  //     small_ptr + small_pos, large_ptr, small_cnt, results);
  // } else {
  //   subsets_trie_kernel_impl<N, Subset<N>::buckets()><<<large_cnt, 32/*, 0, stream*/>>>(
  //     small_ptr + small_pos, large_ptr, small_cnt, results);
  // }
  getLastCudaError("SubsetsImplicitTrieKernel run failed\n"); // TODO: better error message

  checkCudaErrors(cudaMemcpy(ret, results, sizeof(bool) * large_cnt, cudaMemcpyDeviceToHost));
}

template<uint N, bool Proper>
void SubsetsImplicitTrieKernel<N, Proper>::allocate(const Subset<N>* small, uint small_cnt, uint large_cnt) {
  deallocate();
  allocated = true;

  // checkCudaErrors(cudaStreamCreate(&stream));

  checkCudaErrors(cudaMalloc((void **) &small_ptr, sizeof(Subset<N>) * small_cnt));
  checkCudaErrors(cudaMalloc((void **) &large_ptr, sizeof(Subset<N>) * large_cnt));
  checkCudaErrors(cudaMalloc((void **) &results, sizeof(bool) * large_cnt));

  checkCudaErrors(cudaMemcpy(small_ptr, small, sizeof(Subset<N>) * small_cnt, cudaMemcpyHostToDevice));
}

template<uint N, bool Proper>
void SubsetsImplicitTrieKernel<N, Proper>::deallocate() {
  if (!allocated) {
    return;
  }

  checkCudaErrors(cudaFree(small_ptr));
  checkCudaErrors(cudaFree(large_ptr));
  checkCudaErrors(cudaFree(results));

  // checkCudaErrors(cudaStreamDestroy(stream));

  allocated = false;
}

template class SubsetsImplicitTrieKernel<AUT_N, false>;
template class SubsetsImplicitTrieKernel<AUT_N, true>;

} // namespace synchrolib

#undef BLK