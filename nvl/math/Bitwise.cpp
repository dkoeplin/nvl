#include "nvl/math/Bitwise.h"

namespace nvl {

pure F64 ulps(const F64 x, const U64 n) {
    // Since `epsilon()` is the gap size (ULP, unit in the last place)
    // of floating-point numbers in interval [1, 2), we can scale it to
    // the gap size of 2^e, where `e` is the exponent of x
    const F64 m = std::fabs(x);

    // Subnormal numbers have fixed exponent, which is `min_exponent - 1`.
    const int exp = m < std::numeric_limits<F64>::min() ? std::numeric_limits<F64>::min_exponent - 1 : std::ilogb(m);
    return n * std::ldexp(std::numeric_limits<F64>::epsilon(), exp);
}

} // namespace nvl
