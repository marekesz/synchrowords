#pragma once
#include <jitdefines.hpp>

#include <synchrolib/utils/general.hpp>
#include <synchrolib/data_structures/subset.hpp>
#include <synchrolib/data_structures/preprocessed_transition.hpp>

namespace synchrolib {

template<uint N, uint K>
void preprocessed_transition_kernel(const PreprocessedTransition<N, K>& trans, const Subset<N>* from, Subset<N>* to, size_t cnt);

} // namespace synchrolib