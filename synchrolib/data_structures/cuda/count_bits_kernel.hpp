#pragma once
#include <jitdefines.hpp>

#include <synchrolib/utils/general.hpp>
#include <synchrolib/data_structures/subset.hpp>

namespace synchrolib {

template<uint N>
void count_bits_kernel(const Subset<N>* subsets, int* acc, size_t cnt);

} // namespace synchrolib