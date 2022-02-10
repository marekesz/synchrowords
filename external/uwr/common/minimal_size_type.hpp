#pragma once

#include <limits>
#include "memory.hpp"

namespace uwr::mem {

template<len_t C>
struct minimal_size_type {
    using type = typename std::conditional_t<
                    C <= std::numeric_limits<std::uint_fast8_t>::max(),
                    std::uint_fast8_t,
                    typename std::conditional_t<
                        C <= std::numeric_limits<std::uint_fast16_t>::max(),
                        std::uint_fast16_t,
                        std::conditional_t<
                            C <= std::numeric_limits<std::uint_fast32_t>::max(),
                            std::uint_fast32_t,
                            std::uint_fast64_t
                        >
                    >
                >;
};

} // namespace uwr::mem