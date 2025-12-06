#pragma once

#include <bit>
#include <cmath>

#include "nvl/geo/Tuple.h"
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

/// Returns the number of bits required to represent the given unsigned number.
pure expand U64 bit_width(const U64 v) { return std::bit_width(v); }

pure expand U64 ceil_div(const U64 a, const U64 b) { return (a + b - 1) / b; }

/// Returns a FP64 number representing N ULPs scaled to the exponent used for x.
/// The result should be that x - ulps(x, 1) should (usually?) be the next representable number just below x.
pure F64 ulps(F64 x, U64 n);

namespace detail {

// Number of combinations with even parity (even numbers of 1s) is 2^(N-1), e.g.
// 00, 11
// 000, 011, 101, 110
// 0000, 0011, 0101, 0110, 1001, 1010, 1100, 1111
template <U64 N>
constexpr Tuple<1 << (N - 1), U64> even_parity_list() {
    static_assert(N > 0, "N must be greater than 0");
    Tuple<1 << (N - 1), U64> list;
    U64 index = 0;
    for (U64 i = 0; i < (1 << N); ++i) {
        if (std::popcount(i) % 2 == 0) {
            list[index] = i;
            ++index;
        }
    }
    return list;
}

} // namespace detail

/// Returns all combinations of N bits with even parity (an even number of 1s).
template <U64 N>
const Tuple<1 << (N - 1), U64> &even_parity() {
    static_assert(N > 0, "N must be greater than 0");
    static constexpr auto list = detail::even_parity_list<N>();
    return list;
}

} // namespace nvl
