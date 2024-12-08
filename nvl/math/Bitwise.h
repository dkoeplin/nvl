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

/// Returns a FP64 number representing N ULPs scaled to the exponent used for x.
/// The result should be that x - ulps(x, 1) should (usually?) be the next representable number just below x.
pure inline F64 ulps(const F64 x, const U64 n) {
    // Since `epsilon()` is the gap size (ULP, unit in the last place)
    // of floating-point numbers in interval [1, 2), we can scale it to
    // the gap size of 2^e, where `e` is the exponent of x

    // If `x` and `y` have different gap sizes (which means they have
    // different exponents), we take the smaller one. Taking the bigger
    // one is also reasonable, I guess.
    const F64 m = std::fabs(x);

    // Subnormal numbers have fixed exponent, which is `min_exponent - 1`.
    const int exp = m < std::numeric_limits<F64>::min() ? std::numeric_limits<F64>::min_exponent - 1 : std::ilogb(m);
    return n * std::ldexp(std::numeric_limits<F64>::epsilon(), exp);
}

} // namespace nvl
