#pragma once

#include "nvl/geo/Face.h"
#include "nvl/geo/Line.h"
#include "nvl/geo/Triangle.h"
#include "nvl/geo/Tuple.h"
#include "nvl/geo/Volume.h"
#include "nvl/macros/Aliases.h"

namespace nvl {

template <U64 N>
struct Intersect {
    Vec<N> pt;               // Location of intersection
    F64 dist = 0;            // Distance from the line start point (a)
    Maybe<Face> face = None; // Face of intersection
};

/// Returns the point closest to the line's `a` which overlaps with the given box, if one exists.
template <U64 N, typename Concrete>
pure Maybe<Intersect<N>> intersect(const AbstractLine<N, Concrete> &line, const Box<N> &box) {
    // General equation for an ND line is:
    //   a(x - x0) = b(y - y0) = c(z - z0) = ...
    // If e.g. x is held constant, can determine any other dimension's value using e.g.:
    //   (y1 - y0)/(x1 - x0) = m
    // (m here is the slope of y/x, e.g. a/b above)
    //   y = m*(x2 - x0) + y0

    if (box.empty())
        return None;

    const Vec<N> &a = line.a();
    // const Vec<N> &b = line.b();
    if (box.contains(a)) {
        Intersect<N> result;
        result.pt = a;
        result.dist = 0;
        result.face = None;
        return result;
    }

    const F64 len = line.length();
    const Vec<N> min = real(box.min);
    const Vec<N> max = box.max_f64();

    F64 closest_dist = len;
    Vec<N> closest_pt;
    Maybe<Face> closest_face = None;
    // Iterate over faces of the box
    // If the line intersects with the box, and the point a is outside the box,
    // the closest point to a should be on one of the faces of the box.
    for (U64 d = 0; d < N * 2; ++d) {
        const Vec<N> &vec = d & 0x1 ? max : min;
        if (const auto pt = line.interpolate(d / 2, vec[d / 2])) {
            if (pt->distance >= 0 && pt->distance < closest_dist && box.contains(pt->point)) {
                closest_dist = pt->distance;
                closest_pt = pt->point;
                closest_face = Face(d & 0x1 ? Dir::Pos : Dir::Neg, d);
            }
        }
    }
    if (closest_face.has_value())
        return Intersect<N>{.pt = closest_pt, .face = closest_face, .dist = closest_dist};
    return None;
}

/// Returns true if this line segment intersects the given box.
template <U64 N, typename Concrete>
pure bool intersects(const AbstractLine<N, Concrete> &line, const Box<N> &box) {
    return intersect(line, box).has_value();
}

} // namespace nvl
