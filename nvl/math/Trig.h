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

/// Returns the result of rotating point [p] around the origin.
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

/// Returns the result of rotating point [p] around point [x].
template <U64 N, typename T>
pure Vec<N> rotate(const Tuple<N, T> &p, const Rotation<N> &rotation, const Vec<N> &x) {
    // Just translate the reference frame such that x is at the origin, rotate, then translate back.
    Vec<N> r = real(p) - x;
    return rotate(r, rotation) + x;
}

/// Returns the rotation (polar coordinates) for a vector from the origin to the given point.
template <U64 N, typename T>
pure Rotation<N> get_rotation(const Tuple<N, T> &p) {
    Rotation<N> rotation;
    Vec<N> v = real(p);
    for (const Plane &plane : Plane::all<N>()) {
        const F64 x = v[plane.axis0];
        const F64 y = v[plane.axis1];
        rotation[plane] = atan<Deg>(x, y);
    }
    return rotation;
}

} // namespace nvl
