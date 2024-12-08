#pragma once

#include "nvl/geo/Face.h"
#include "nvl/geo/Tuple.h"
#include "nvl/macros/Aliases.h"

namespace nvl {

template <U64 N>
struct Intersect {
    Vec<N> pt;               // Location of intersection
    F64 dist = 0;            // Distance from the line start point (a)
    Maybe<Face> face = None; // Face of intersection
};

} // namespace nvl
