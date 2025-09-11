#pragma once

#include "nvl/geo/Tuple.h"
#include "nvl/macros/Aliases.h"

namespace nvl {

template <U64 N>
class Triangle;

template <U64 N>
pure expand Triangle<N> make_triangle(const Vec<N> &a, const Vec<N> &b, const Vec<N> &c);

/**
 * @class AbstractTriangle
 * @brief A triangle in N-dimensional space formed by three points: A, B, and C.
 * @tparam N - Number of dimensions
 */
template <U64 N, typename Concrete>
abstract class AbstractTriangle {
public:
    /// Returns a copy of this line, shifted by x.
    template <typename T>
    pure constexpr Triangle<N> operator+(const Tuple<N, T> &x) const {
        const auto shift = real(x);
        return make_triangle(a() + shift, b() + shift, c() + shift);
    }

    template <typename T>
    pure constexpr Triangle<N> operator-(const Tuple<N, T> &x) const {
        const auto shift = real(x);
        return make_triangle(a() - shift, b() - shift, c() - shift);
    }

    pure expand const Vec<N> &a() const { return static_cast<const Concrete *>(this)->a_; }
    pure expand const Vec<N> &b() const { return static_cast<const Concrete *>(this)->b_; }
    pure expand const Vec<N> &c() const { return static_cast<const Concrete *>(this)->c_; }

    pure std::string to_string() const {
        std::stringstream ss;
        return ss << "Triangle{" << a() << ", " << b() << ", " << c() << "}";
    }
};

/**
 * @class Triangle
 * @brief A triangle in N-dimensional space over points {A, B, C}.
 * @tparam N - Number of dimensions.
 */
template <U64 N>
class Triangle : AbstractTriangle<N, Triangle<N>> {
public:
    constexpr Triangle() = default;
    constexpr Triangle(const Vec<N> &a, const Vec<N> &b, const Vec<N> &c) : a_(a), b_(b), c_(c) {}

private:
    Vec<N> a_;
    Vec<N> b_;
    Vec<N> c_;
};

/**
 * @class TriangleView
 * @brief A triangle in N-dimensional space implemented as a view over points {A, B, C}.
 * @tparam N - Number of dimensions.
 */
template <U64 N>
class TriangleView : AbstractTriangle<N, TriangleView<N>> {
public:
    constexpr TriangleView(const Vec<N> &a, const Vec<N> &b, const Vec<N> &c) : a_(a), b_(b), c_(c) {}

private:
    const Vec<N> &a_;
    const Vec<N> &b_;
    const Vec<N> &c_;
};

template <U64 N>
pure expand Triangle<N> make_triangle(const Vec<N> &a, const Vec<N> &b, const Vec<N> &c) {
    return Triangle(a, b, c);
}

template <U64 N>
std::ostream &operator<<(std::ostream &os, const Triangle<N> &t) {
    return os << "Triangle{" << t.a() << ", " << t.b() << ", " << t.c() << "}";
}

template <U64 N>
std::ostream &operator<<(std::ostream &os, const TriangleView<N> &t) {
    return os << "Triangle{" << t.a() << ", " << t.b() << ", " << t.c() << "}";
}

} // namespace nvl
