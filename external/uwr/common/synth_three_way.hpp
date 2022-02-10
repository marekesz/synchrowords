#pragma once

#if CPP_ABOVE_17

#include <compare>

#include "define.hpp"

namespace uwr::mem {

static constexpr inline struct synth_three_way_t {
    template<class T, std::totally_ordered_with<T> U>
    auto operator()(const T& lhs, const U& rhs) {
        if constexpr (std::three_way_comparable_with<T, U>)
            return lhs <=> rhs;
        else {
            if (lhs == rhs)
                return std::strong_ordering::equal;
            else if (lhs < rhs)
                return std::strong_ordering::less;
            else
                return std::strong_ordering::greater;
        }
    }
} synth_three_way;

} // namespace uwr::mem

#endif
