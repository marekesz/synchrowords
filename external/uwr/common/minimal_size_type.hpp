#pragma once

#include <limits>
#include <type_traits>

#include "types.hpp"

namespace uwr::mem {

template<uwr::u64 C>
struct minimal_size_type {
   using type = typename std::conditional_t<
      C <= std::numeric_limits<uwr::u8>::max(),
      uwr::u8,
      typename std::conditional_t<
         C <= std::numeric_limits<uwr::u16>::max(),
         uwr::u16,
         std::conditional_t<
            C <= std::numeric_limits<uwr::u32>::max(),
            uwr::u32,
            uwr::u64
         >
      >
   >;
};

} // namespace uwr::mem
