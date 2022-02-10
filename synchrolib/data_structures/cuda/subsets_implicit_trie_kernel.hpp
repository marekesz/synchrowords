#pragma once
#include <jitdefines.hpp>

#include <synchrolib/utils/general.hpp>
#include <synchrolib/data_structures/subset.hpp>
#include <memory>

namespace synchrolib {

template<uint N, bool Proper=false>
class SubsetsImplicitTrieKernel : public NonCopyable, public NonMovable {
public:
  SubsetsImplicitTrieKernel();
  ~SubsetsImplicitTrieKernel();

  void run(
      uint small_pos,
      uint small_cnt,
      const Subset<N>* large,
      uint large_cnt,
      bool* ret);

  void allocate(const Subset<N>* small, uint small_cnt, uint large_cnt);
  void deallocate();

private:
  bool allocated;

  // cudaStream_t stream;

  Subset<N>* small_ptr;
  Subset<N>* large_ptr;
  bool* results;
};


} // namespace synchrolib