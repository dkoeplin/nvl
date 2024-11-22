#pragma once

#include "nvl/geo/Face.h"
#include "nvl/geo/Vec.h"
#include "nvl/macros/Aliases.h"

namespace nvl {

template <U64 N>
struct Intersect {
    Vec<N> pt;    // Location of intersection
    Face face;    // Face of intersection
    F64 dist = 0; // Distance from the line start point (a)
};

} // namespace nvl
