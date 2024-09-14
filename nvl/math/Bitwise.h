#pragma once

#include <bit>

#include "nvl/macros/Aliases.h"
#include "nvl/macros/Inline.h"

namespace nvl {

/// Returns the result of rotating the 64-bit value "v" left by N bits.
template <U64 bits>
INLINE U64 rotate_left(const U64 v) {
    const U64 left = v << bits;
    const U64 right = v >> (64 - bits);
    return left | right;
}

INLINE U64 ceil_log2(const U64 v) { return std::bit_width(v); }

} // namespace nvl
