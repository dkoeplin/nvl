#pragma once

#include "nvl/geo/Intersect.h"
#include "nvl/geo/Tuple.h"
#include "nvl/geo/Volume.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/Pure.h"

namespace nvl {

/**
 * @struct Line
 * @brief A line segment between points a and b.
 * @tparam N
 */
template <U64 N>
class Line {
public:
    constexpr Line() = default;
    constexpr Line(const Vec<N> &x, const Vec<N> &y) : a_(x), b_(y) {}

    /// Returns the point closest to `a` which overlaps with the given box, if one exists.
    pure Maybe<Intersect<N>> intersect(const Box<N> &box) const;

    /// Returns true if this line segment intersects the given box.
    pure bool intersects(const Box<N> &box) const { return intersect(box).has_value(); }

    /// Returns the point on the line with the same slope as this line segment which is `dist` away from point a.
    /// The resulting point may not be on this line segment itself.
    pure Vec<N> interpolate(F64 dist) const;

    /// Returns the point on the line with the same slope as this line segment where dimension `dim` has the given `x`.
    /// The resulting point may not be on this line segment itself.
    pure Maybe<Intersect<N>> interpolate(U64 dim, F64 x) const;

    /// Returns the point on this line segment where dimension `dim` has the value `x`.
    pure Maybe<Vec<N>> where(U64 dim, F64 x) const;

    pure const Vec<N> &a() const { return a_; }
    pure const Vec<N> &b() const { return b_; }
    pure F64 length() const {
        if (!length_.has_value()) {
            F64 len = 0;
            const Vec<N> diff = b_ - a_;
            for (U64 i = 0; i < N; ++i) {
                len += diff[i] * diff[i];
            }
            length_ = std::sqrt(len);
        }
        return *length_;
    }

    pure const Vec<N> &slope() const {
        if (!slope_.has_value()) {
            slope_ = (b_ - a_) / length();
        }
        return *slope_;
    }

    pure std::string to_string() const {
        std::stringstream ss;
        return ss << "Line{" << a_ << ", " << b_ << "}";
    }

private:
    Vec<N> a_;
    Vec<N> b_;
    mutable Maybe<F64> length_ = None;   // Length between a and b, lazily computed
    mutable Maybe<Vec<N>> slope_ = None; // Slope between a and b, lazily computed
};

// General equation for an ND line is:
//   a(x - x0) = b(y - y0) = c(z - z0) = ...
// If e.g. x is held constant, can determine any other dimension's value using e.g.:
//   (y1 - y0)/(x1 - x0) = m
// (m here is the slope of y/x, e.g. a/b above)
//   y = m*(x2 - x0) + y0
template <U64 N>
Maybe<Intersect<N>> intersect(const Line<N> &line, const Box<N> &box) {
    if (box.empty())
        return None;

    const Vec<N> &a = line.a();
    if (box.contains(a)) {
        Intersect<N> result;
        result.pt = a;
        result.dist = 0;
        result.face = None;
        return result;
    }

    const F64 len = line.length();
    Maybe<Intersect<N>> closest = None;
    // Iterate over faces of the box
    // If the line intersects with the box, and the point a is outside the box,
    // the closest point to a should be on one of the faces of the box.
    for (const auto &edge : box.faces()) {
        // A face is an N-dimensional surface where one of the dimensions (d) has a fixed value.
        const Box<N> &f = edge.bbox();
        const U64 d = edge.dim;
        const I64 x = (edge.dir == Dir::Neg) ? f.min[d] : f.end[d] - 1;
        const Maybe<Intersect<N>> pt = line.interpolate(d, x);
        if (pt && pt->dist >= 0 && pt->dist <= len && box.contains(pt->pt)) {
            if (!closest.has_value() || pt->dist < closest->dist) {
                closest = pt;
                closest->face = edge.face();
            }
        }
    }
    return closest;
}

template <U64 N>
Maybe<Intersect<N>> Line<N>::intersect(const Box<N> &box) const {
    return nvl::intersect(*this, box);
}

template <U64 N>
pure Vec<N> Line<N>::interpolate(const F64 dist) const {
    return a_ + slope() * dist;
}

template <U64 N>
pure Maybe<Intersect<N>> Line<N>::interpolate(const U64 dim, const F64 x) const {
    ASSERT(dim < N, "Invalid dimension " << dim << " for rank " << N << " line.");
    if (a_[dim] == b_[dim])
        return None;

    // If the line has a non-zero slope in dimension d, calculate the slope with all other dimensions.
    // Use this to calculate the point on the line where dimension d would intersect with this face.
    const Vec<N> slope = b_ - a_;
    const F64 dx = slope[dim];
    const F64 x0 = a_[dim];
    const F64 delta_x = x - x0;

    Intersect<N> intersect;
    Dir dir = Dir::Pos;
    for (U64 i = 0; i < N; ++i) {
        const F64 y0 = a_[i];
        const F64 dy = slope[i];
        const F64 m = dy / dx;
        const F64 delta_y = m * delta_x;
        intersect.pt[i] = delta_y + y0;
        if (std::signbit(delta_y) != std::signbit(dy)) {
            dir = Dir::Neg;
        }
    }
    intersect.pt[dim] = x;
    intersect.dist = dir * intersect.pt.dist(a_);
    return intersect;
}

template <U64 N>
pure Maybe<Vec<N>> Line<N>::where(const U64 dim, const F64 x) const {
    if (auto intersect = interpolate(dim, x); intersect && intersect->dist >= 0 && intersect->dist <= length()) {
        return intersect->pt;
    }
    return None;
}

template <U64 N>
std::ostream &operator<<(std::ostream &os, const Line<N> &line) {
    return os << "Line{" << line.a() << ", " << line.b() << "}";
}

} // namespace nvl
