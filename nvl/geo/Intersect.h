#pragma once

#include "nvl/data/WalkResult.h"
#include "nvl/geo/Face.h"
#include "nvl/geo/Line.h"
#include "nvl/geo/RBox.h"
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

/// Returns the point closest to the line's [a] which overlaps with the given box, if one exists.
template <U64 N, typename T, typename Concrete, bool closest = true>
pure Maybe<Intersect<N>> intersect(const AbstractLine<N, Concrete> &line, const Volume<N, T> &box) {
    // General equation for an ND line is:
    //   a(x - x0) = b(y - y0) = c(z - z0) = ...
    // If e.g. x is held constant, can determine any other dimension's value using e.g.:
    //   (y1 - y0)/(x1 - x0) = m
    // (m here is the slope of y/x, e.g. a/b above)
    //   y = m*(x2 - x0) + y0
    return_if(box.empty(), None);

    const Vec<N> &a = line.a();
    return_if(box.contains(a), Intersect<N>{.pt = a, .dist = 0, .face = None});

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
                return_if(!closest, Intersect<N>{.pt = closest_pt, .face = closest_face, .dist = closest_dist});
            }
        }
    }
    if (closest_face.has_value())
        return Intersect<N>{.pt = closest_pt, .face = closest_face, .dist = closest_dist};
    return None;
}

/// Returns true if this line segment intersects the given box.
template <U64 N, typename T, typename Concrete>
pure bool intersects(const AbstractLine<N, Concrete> &line, const Volume<N, T> &box) {
    // Don't necessarily need to find the closest intersection, just need to test for existence.
    return intersect<N, T, Concrete, /*closest*/ false>(line, box).has_value();
}

namespace detail {

template <U64 N, bool closest>
pure Maybe<Intersect<N>> intersect_lhs(const RBox<N> &a, const RBox<N> &b) {
    // Rotate the reference frame such that A has zero rotation
    const Volume<N, F64> a_raw = a.raw_volume();
    const RBox<N> b_rot = b.rotated(-a.rotation(), a.center());

    // TODO: The minimization here isn't very well defined.
    // Case 1: There exists at least one edge of B which intersects with a face of A.
    //         Find the intersection which has minimal distance to the corresponding face of A
    Maybe<Intersect<N>> result = None;
    b_rot.walk_lines([&](const LineView<N> &line) {
        if (auto intersection = intersect<N, F64, LineView, closest>(line, a_raw)) {
            if (!result.has_value() || intersection->dist < result->dist) {
                result = intersection;
                return_if(!closest || result->dist == 0, WalkResult::kExit);
            }
        }
    });
    if (result.has_value()) {
        return Intersect<N>{
            .pt = rotate(result->pt, a.rotation(), a.center()), .dist = result->dist, .face = result->face};
    }

    // Case 2: No edges intersect with any faces, but all points in B lie in A.
    if (const Vec<N> &b_pt = b_rot.points()[0]; a_raw.contains(b_pt)) {
        return Intersect<N>{.pt = rotate(b_pt, a.rotation(), a.center()), .dist = 0, .face = None};
    }

    return None;
}

} // namespace detail

/// Returns the face and point on [a] which overlaps with [b] with minimal distance from the corresponding face of [a].
template <U64 N, bool closest = true>
pure Maybe<Intersect<N>> intersect(const RBox<N> &a, const RBox<N> &b) {
    return_if(a.empty() || b.empty(), None);

    const auto a_b = detail::intersect_lhs<N, closest>(a, b);
    return_if(a_b, a_b);

    return detail::intersect_lhs<N, closest>(b, a);
}

/// Returns true if boxes [a] and [b] overlap.
template <U64 N>
bool intersects(const RBox<N> &a, const RBox<N> &b) {
    return intersect<N, /*closest*/ false>(a, b).has_value();
}

} // namespace nvl
