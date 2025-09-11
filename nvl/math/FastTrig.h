#pragma once

#include "nvl/data/List.h"
#include "nvl/data/Range.h"
#include "nvl/geo/Tuple.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/Expand.h"
#include "nvl/macros/Implicit.h"
#include "nvl/macros/Pure.h"
#include "nvl/math/Combinations.h"

namespace nvl::fasttrig {

class Plane {
public:
    template <U64 N>
    static const List<Plane> &all() {
        static const List<Plane> list = make_list<N>();
        return list;
    }

    U64 axis0;
    U64 axis1;
    U64 index;

private:
    explicit constexpr Plane(const U64 axis0, const U64 axis1, const U64 index)
        : axis0(axis0), axis1(axis1), index(index) {}

    template <U64 N>
    static List<Plane> make_list() {
        List<Plane> list;
        U64 index = 0;
        for (U64 a0 = 0; a0 < N; ++a0) {
            for (U64 a1 = a0 + 1; a1 < N; ++a1) {
                list.push_back(Plane(a0, a1, index));
                ++index;
            }
        }
        return list;
    }
};

static const Plane kPlane2D = Plane::all<2>()[0];

namespace Axis3D {
static const Plane &kXY = Plane::all<3>()[0];
static const Plane &kXZ = Plane::all<3>()[1];
static const Plane &kYZ = Plane::all<3>()[2];
} // namespace Axis3D

/**
 * @class Deg
 * @brief Wrapper class around a fixed-point representation for degrees.
 */
class Deg {
public:
    constexpr Deg() : d(0) {}
    constexpr implicit Deg(const I64 degrees) : d(degrees) {}
    pure constexpr expand implicit operator I64() const { return d; }

    expand Deg &operator+=(const Deg rhs) {
        d += rhs.d;
        return *this;
    }
    expand Deg &operator-=(const Deg rhs) {
        d -= rhs.d;
        return *this;
    }

    pure constexpr expand Deg operator+(const Deg rhs) const { return {d + rhs.d}; }
    pure constexpr expand Deg operator-(const Deg rhs) const { return {d - rhs.d}; }

private:
    I64 d;
};

template <U64 N>
struct Rot {
    static constexpr U64 R = combinations(N, 2);

    constexpr Rot() = default;
    constexpr Rot(std::initializer_list<Deg> list) {
        U64 index = 0;
        for (auto x : list) {
            theta[index] = x;
        }
    }

    pure constexpr expand Deg &operator[](const Plane &plane) { return theta[plane.index]; }
    pure constexpr expand const Deg &operator[](const Plane &plane) const { return theta[plane.index]; }

    Deg theta[R] = {};
};

pure F64 sin(Deg deg);
pure F64 cos(Deg deg);
pure F64 tan(Deg deg);

pure Deg atan(I64 x, I64 y);

/// Returns the result of rotating x around the origin.
template <U64 N, typename T>
pure Vec<N> rotate(const Tuple<N, T> &p, const Rot<N> &rotation) {
    Vec<N> v = real(p);
    const F64 len = v.magnitude();
    for (const Plane &plane : Plane::all<N>()) {
        F64 &x = v[plane.axis0];
        F64 &y = v[plane.axis1];
        const Deg theta = rotation[plane] + atan(x, y);
        x = len * cos(theta);
        y = len * sin(theta);
    }
    return v;
}

} // namespace nvl::fasttrig
