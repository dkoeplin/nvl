#pragma once

#include "nvl/geo/Tuple.h"
#include "nvl/geo/Volume.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/Pure.h"
#include "nvl/macros/ReturnIf.h"

namespace nvl {

template <U64 N>
class Line;

template <U64 N>
pure expand Line<N> make_line(const Vec<N> &a, const Vec<N> &b);

template <U64 N>
struct InterpolatedPoint {
    Vec<N> point;
    F64 distance;
};

/**
 * @class AbstractLine
 * @brief Am abstract line segment in N-dimensional space described by points A and B.
 * @tparam N - Number of dimensions.
 * @tparam Concrete - Concrete subclass.
 */
template <U64 N, typename Concrete>
abstract class AbstractLine {
public:
    /// Returns a copy of this line, shifted by x.
    template <typename T>
    pure constexpr Line<N> operator+(const Tuple<N, T> &x) const {
        const Tuple<N, T> shift = real(x);
        return make_line(a() + shift, b() + shift);
    }

    template <typename T>
    pure constexpr Line<N> operator-(const Tuple<N, T> &x) const {
        const Tuple<N, T> shift = real(x);
        return make_line(a() - shift, b() - shift);
    }

    /// Returns the point on the line with the same slope as this line segment which is `dist` away from point a.
    /// The resulting point may not be on this line segment itself.
    pure expand Vec<N> interpolate(F64 dist) const { return a() + dist * ((b() - a()) / length()); }

    /// Returns the point on the line with the same slope as this line segment where dimension `dim` has the given
    /// `x`. The resulting point may not be on this line segment itself.
    pure Maybe<InterpolatedPoint<N>> interpolate(U64 dim, F64 x) const;

    /// Returns the point on this line segment where dimension `dim` has the value `x`.
    pure Maybe<Vec<N>> where(U64 dim, F64 x) const;

    pure expand const Vec<N> &a() const { return static_cast<const Concrete *>(this)->a_; }
    pure expand const Vec<N> &b() const { return static_cast<const Concrete *>(this)->b_; }

    pure F64 length() const {
        if (!length_.has_value()) {
            F64 len = 0;
            const Vec<N> diff = b() - a();
            for (U64 i = 0; i < N; ++i) {
                len += diff[i] * diff[i];
            }
            length_ = std::sqrt(len);
        }
        return *length_;
    }

    pure const Vec<N> &slope() const {
        if (!slope_.has_value()) {
            slope_ = (b() - a()) / length();
        }
        return *slope_;
    }

    pure std::string to_string() const {
        std::stringstream ss;
        ss << "Line{" << a() << ", " << b() << "}";
        return ss.str();
    }

private:
    mutable Maybe<F64> length_ = None;   // Length between a and b, lazily computed
    mutable Maybe<Vec<N>> slope_ = None; // Slope between a and b, lazily computed
};

/**
 * @class Line
 * @brief A line segment in N-dimension space between points A and B.
 * @tparam N - Number of dimensions.
 */
template <U64 N>
class Line : public AbstractLine<N, Line<N>> {
public:
    constexpr Line() = default;
    constexpr Line(const Vec<N> &a, const Vec<N> &b) : a_(a), b_(b) {}

protected:
    friend class AbstractLine<N, Line>;
    Vec<N> a_;
    Vec<N> b_;
};

/**
 * @class LineView
 * @brief A line segment implemented as a view over points {A, B}.
 * @tparam N - Number of dimensions.
 */
template <U64 N>
class LineView : public AbstractLine<N, LineView<N>> {
public:
    constexpr LineView(const Vec<N> &a, const Vec<N> &b) : a_(a), b_(b) {}
    explicit operator Line<N>() { return Line(a_, b_); }

protected:
    friend class AbstractLine<N, LineView>;
    const Vec<N> &a_;
    const Vec<N> &b_;
};

template <U64 N>
pure expand Line<N> make_line(const Vec<N> &a, const Vec<N> &b) {
    return Line(a, b);
}

template <U64 N, typename Concrete>
pure Maybe<InterpolatedPoint<N>> AbstractLine<N, Concrete>::interpolate(const U64 dim, const F64 x) const {
    const F64 x0 = a()[dim];
    const F64 x1 = b()[dim];
    return_if(x0 == x1, None);
    // Infer the distance from x0 first, then interpolate from that.
    const F64 dist = length() * (x - x0) / (x1 - x0);
    InterpolatedPoint<N> result;
    result.point = interpolate(dist);
    result.point[dim] = x;
    result.distance = dist;
    return result;
}

template <U64 N, typename Concrete>
pure Maybe<Vec<N>> AbstractLine<N, Concrete>::where(const U64 dim, const F64 x) const {
    if (auto intersect = interpolate(dim, x); intersect && intersect->dist >= 0 && intersect->dist <= length()) {
        return intersect->pt;
    }
    return None;
}

template <U64 N>
std::ostream &operator<<(std::ostream &os, const Line<N> &line) {
    return os << line.to_string();
}

template <U64 N>
std::ostream &operator<<(std::ostream &os, const LineView<N> &line) {
    return os << line.to_string();
}

} // namespace nvl
