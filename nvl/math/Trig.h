#pragma once

#include "nvl/geo/Tuple.h"
#include "nvl/macros/Aliases.h"
#include "nvl/math/Deg.h"
#include "nvl/math/Plane.h"
#include "nvl/math/Rotation.h"

namespace nvl {

template <typename D>
pure expand D atan(const F64 x, const F64 y) {
    return D(std::round(std::atan(y / x) * kRad2Deg) + (x < 0 ? 180 : 0));
}

/// Returns the result of rotating x around the origin.
template <U64 N, typename T>
pure Vec<N> rotate(const Tuple<N, T> &p, const Rotation<N> &rotation) {
    Vec<N> v = real(p);
    const F64 len = v.magnitude();
    for (const Plane &plane : Plane::all<N>()) {
        F64 &x = v[plane.axis0];
        F64 &y = v[plane.axis1];
        const Deg theta = rotation[plane] + atan<Deg>(x, y);
        x = len * cos(theta);
        y = len * sin(theta);
    }
    return v;
}

} // namespace nvl
