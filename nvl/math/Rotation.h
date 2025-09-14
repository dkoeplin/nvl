#pragma once

#include <string>

#include "nvl/geo/Tuple.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/Expand.h"
#include "nvl/macros/Pure.h"
#include "nvl/math/Combinations.h"
#include "nvl/math/Deg.h"
#include "nvl/math/Plane.h"

namespace nvl {

/**
 * @struct Rotation
 * @brief Describes rotation in an N-dimensional space.
 * @tparam N - Number of dimensions.
 */
template <U64 N>
struct Rotation {
    static constexpr U64 R = combinations(N, 2);

    static const Rotation zero;

    constexpr Rotation() = default;
    Rotation(const std::initializer_list<Deg> list) {
        U64 index = 0;
        for (auto x : list) {
            theta[index] = x;
        }
    }
    constexpr explicit Rotation(const Tuple<R, Deg> &theta) : theta(theta) {}

    pure Deg &operator[](const Plane &plane) { return theta[plane.index]; }
    pure const Deg &operator[](const Plane &plane) const { return theta[plane.index]; }

    Rotation &operator+=(const Rotation &rhs) {
        theta += rhs.theta;
        return *this;
    }
    Rotation &operator-=(const Rotation &rhs) {
        theta -= rhs.theta;
        return *this;
    }

    pure constexpr expand Rotation operator+(const Rotation &rhs) const { return Rot(theta + rhs.theta); }
    pure constexpr expand Rotation operator-(const Rotation &rhs) const { return Rot(theta - rhs.theta); }

    pure std::string to_string() const { return theta.to_string(); }

    Tuple<R, Deg> theta;
};

template <U64 N>
const Rotation<N> Rotation<N>::zero = Rotation(Tuple<R, Deg>::zero);

template <U64 N>
std::ostream &operator<<(std::ostream &os, const Rotation<N> &rotation) {
    return os << rotation.to_string();
}

} // namespace nvl
