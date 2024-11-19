#pragma once

#include "nvl/geo/Tuple.h"
#include "nvl/macros/Aliases.h"

namespace nvl {

/**
 * @class Vec
 * @brief A tuple of N floating point values.
 * @tparam N - Number of elements in this tuple.
 */
template <U64 N>
class Vec : public Tuple<N, F64, Vec<N>> {
public:
};

} // namespace nvl