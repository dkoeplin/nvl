#pragma once

#include "nox/macros/Aliases.h"
#include "nox/macros/Inline.h"

namespace nox {

/// Returns the result of rotating the 64-bit value "v" left by N bits.
template <U64 bits> INLINE U64 rotate_left(const U64 v) {
    const U64 left = v << bits;
    const U64 right = v >> (64 - bits);
    return left | right;
}

} // namespace nox
