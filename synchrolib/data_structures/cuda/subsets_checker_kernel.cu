#include <synchrolib/data_structures/cuda/subsets_checker_kernel.hpp>

#include <external/cuda/helper_cuda.h>
#include <cuda.h>

namespace {

__global__ void check_subset_intersection_kernel_impl_(synchrolib::uint64* small, synchrolib::uint64* large, bool* results, int small_cnt, int large_cnt, int buckets, int thresh) {
    int ind = threadIdx.x + blockIdx.x * blockDim.x;
    if (ind >= small_cnt) return;

    int my = ind * buckets;

    int cnt = 0;

    for (int i = 0; i < large_cnt; ++i) {
        int they = i * buckets;

        bool good = true;
        for (int j = 0; j < buckets; ++j) {
            synchrolib::uint64 sm = small[my + j];
            if ((sm & large[they + j]) != sm) {
                good = false;
            }
        }

        if (good) {
            cnt++;
        }
    }

    results[ind] = (cnt >= thresh);
}

} // namespace


namespace synchrolib {

std::unique_ptr<bool[]> check_subset_intersection_kernel_impl(const uint64* small, const uint64* large, int small_cnt, int large_cnt, int buckets, int thresh) {
    uint64* small_ptr;
    uint64* large_ptr;
    bool* results;

    checkCudaErrors(cudaMalloc((void **) &small_ptr, sizeof(uint64) * buckets * small_cnt));
    checkCudaErrors(cudaMalloc((void **) &large_ptr, sizeof(uint64) * buckets * large_cnt));
    checkCudaErrors(cudaMalloc((void **) &results, sizeof(bool) * small_cnt));

    checkCudaErrors(cudaMemcpy(small_ptr, small, sizeof(uint64) * buckets * small_cnt, cudaMemcpyHostToDevice));
    checkCudaErrors(cudaMemcpy(large_ptr, large, sizeof(uint64) * buckets * large_cnt, cudaMemcpyHostToDevice));
    checkCudaErrors(cudaMemset(results, 0, sizeof(bool) * small_cnt));

    constexpr int threads = 1024;
    check_subset_intersection_kernel_impl_<<<(small_cnt + threads - 1) / threads, threads>>>(
        small_ptr, large_ptr, results, small_cnt, large_cnt, buckets, thresh);
    getLastCudaError("check_subset_intersection_kernel run failed\n"); // TODO: better error message

    auto results_array = std::make_unique<bool[]>(small_cnt);
    checkCudaErrors(cudaMemcpy(results_array.get(), results, sizeof(bool) * small_cnt, cudaMemcpyDeviceToHost));

    return results_array;
}

void reset_cuda_device_impl() {
    cudaDeviceReset();
}

} // namespace synchrolib