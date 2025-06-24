#pragma once

#include "nvl/geo/Intersect.h"
#include "nvl/geo/Tuple.h"
#include "nvl/geo/Volume.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/Pure.h"
#include "nvl/macros/ReturnIf.h"

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

    template <typename T>
    pure Line operator+(const Tuple<N, T> &rhs) const {
        return Line(a_ + real(rhs), b_ + real(rhs));
    }

    template <typename T>
    pure Line operator-(const Tuple<N, T> &rhs) const {
        return Line(a_ - real(rhs), b_ - real(rhs));
    }

    /// Returns the point closest to `a` which overlaps with the given box, if one exists.
    pure Maybe<Intersect<N>> intersect(const Box<N> &box) const;

    /// Returns true if this line segment intersects the given box.
    pure bool intersects(const Box<N> &box) const { return intersect(box).has_value(); }

    /// Returns the point on the line with the same slope as this line segment which is `dist` away from point a.
    /// The resulting point may not be on this line segment itself.
    pure expand Vec<N> interpolate(F64 dist) const { return a_ + dist * ((b_ - a_) / length()); }

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
pure Maybe<Intersect<N>> intersect(const Line<N> &line, const Box<N> &box) {
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
        const Vec<N> &vec = (d & 0x1) ? max : min;
        if (const auto pt = line.interpolate(d / 2, vec[d / 2])) {
            if (pt->dist >= 0 && pt->dist < closest_dist && box.contains(pt->pt)) {
                closest_dist = pt->dist;
                closest_pt = pt->pt;
                closest_face = Face((d & 0x1) ? Dir::Pos : Dir::Neg, d);
            }
        }
    }
    if (closest_face.has_value())
        return Intersect<N>{.pt = closest_pt, .face = closest_face, .dist = closest_dist};
    return None;
}

template <U64 N>
pure Maybe<Intersect<N>> Line<N>::intersect(const Box<N> &box) const {
    return nvl::intersect(*this, box);
}

template <U64 N>
pure Maybe<Intersect<N>> Line<N>::interpolate(const U64 dim, const F64 x) const {
    const F64 x0 = a_[dim];
    const F64 x1 = b_[dim];
    return_if(x0 == x1, None);
    // Infer the distance from x0 first, then interpolate from that.
    const F64 dist = length() * (x - x0) / (x1 - x0);
    Intersect<N> result;
    result.pt = interpolate(dist);
    result.pt[dim] = x;
    result.dist = dist;
    return result;
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
