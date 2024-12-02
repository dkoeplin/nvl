#pragma once

#include "nvl/geo/Box.h"
#include "nvl/geo/Intersect.h"
#include "nvl/geo/Pos.h"
#include "nvl/geo/Vec.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/Pure.h"

namespace nvl {

/**
 * @struct Line
 * @brief A line segment between points a and b.
 * @tparam N
 */
template <U64 N>
struct Line {
    constexpr Line() = default;
    constexpr Line(const Pos<N> &x, const Pos<N> &y) : a(x), b(y) {}

    /// Returns the point closest to `a` which overlaps with the given box, if one exists.
    pure Maybe<Intersect<N>> intersect(const Box<N> &box) const;

    /// Returns true if this line segment intersects the given box.
    pure bool intersects(const Box<N> &box) const { return intersect(box).has_value(); }

    pure std::string to_string() const {
        std::stringstream ss;
        return ss << "Line{" << a << ", " << b << "}";
    }

    Pos<N> a;
    Pos<N> b;
};

// General equation for an ND line is:
//   a(x - x0) = b(y - y0) = c(z - z0) = ...
// If e.g. x is held constant, can determine any other dimension's value using e.g.:
//   (y1 - y0)/(x1 - x0) = m
// (m here is the slope of y/x, e.g. a/b above)
//   y = m*(x2 - x0) + y0
template <U64 N>
Maybe<Intersect<N>> intersect(const Line<N> &line, const Box<N> &box) {
    const Vec<N> a = line.a.to_vec();
    const Vec<N> b = line.b.to_vec();
    if (box.contains(a)) {
        Intersect<N> result;
        result.pt = a;
        result.dist = 0;
        result.face = Face(Dir::Pos, 0);
        return result;
    }

    Maybe<Intersect<N>> closest = None;
    // Iterate over faces of the box
    // If the line intersects with the box, and the point a is outside the box,
    // the closest point to a should be on one of the faces of the box.
    for (const Edge<N> &edge : box.faces()) {
        // A face is an N-dimensional surface where one of the dimensions (d) has a fixed value.
        const Box<N> &f = edge.bbox();
        const U64 d = edge.dim;
        const I64 x0 = a[d];
        const F64 dx = static_cast<F64>(b[d] - a[d]);
        const I64 x = f.min[d];
        if (a[d] != b[d]) {
            // If the line has a non-zero slope in dimension d, calculate the slope with all other dimensions.
            // Use this to calculate the point on the line where dimension d would intersect with this face.
            Vec<N> pt;
            bool on_segment = true;
            for (U64 i = 0; i < N && on_segment; ++i) {
                const F64 dy = static_cast<F64>(b[i] - a[i]);
                const I64 y0 = a[i];
                const I64 y1 = b[i];
                const F64 m = dy / dx;
                pt[i] = m * (x - x0) + y0;
                on_segment = (pt[i] >= y0 && pt[i] <= y1);
            }
            pt[d] = x; // Should be redundant, but floating point errors
            if (on_segment && box.contains(pt)) {
                const F64 dist = pt.dist(a);
                const bool keep = !closest.has_value() || dist < closest->dist;
                if (!closest.has_value()) {
                    Intersect<N> result;
                    closest = result;
                }
                if (keep) {
                    closest->pt = pt;
                    closest->dist = dist;
                    closest->face = edge.face();
                }
            }
        } else {
            // The line is constant in this dimension, use a different dimension for now?
        }
    }
    return closest;
}

template <U64 N>
Maybe<Intersect<N>> Line<N>::intersect(const Box<N> &box) const {
    return nvl::intersect(*this, box);
}

template <U64 N>
std::ostream &operator<<(std::ostream &os, const Line<N> &line) {
    return os << "Line{" << line.a << ", " << line.b << "}";
}

} // namespace nvl