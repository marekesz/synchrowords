#pragma once
#include <synchrolib/utils/general.hpp>
#include <memory>

namespace synchrolib {

std::unique_ptr<bool[]> check_subset_intersection_kernel_impl(const uint64* small, const uint64* large, int small_cnt, int large_cnt, int buckets, int thresh);
void reset_cuda_device_impl();

} // namespace synchrolib