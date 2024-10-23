#pragma once

#include <bit>

#include "nvl/macros/Aliases.h"
#include "nvl/macros/Expand.h"
#include "nvl/macros/Pure.h"

namespace nvl {

/// Returns the result of rotating the 64-bit value "v" left by N bits.
template <U64 bits>
pure expand U64 rotate_left(const U64 v) {
    const U64 left = v << bits;
    const U64 right = v >> (64 - bits);
    return left | right;
}

/// Returns ceil(log2(v)), which is the same as the number of bits required to represent v.
pure expand U64 ceil_log2(const U64 v) { return std::bit_width(v); }

pure expand U64 ceil_div(const U64 a, const U64 b) { return (a + b - 1) / b; }

} // namespace nvl
