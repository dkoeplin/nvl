#pragma once

#include "nvl/geo/Tuple.h"
#include "nvl/geo/Vec.h"
#include "nvl/math/Grid.h"

namespace nvl {

/**
 * @class Pos
 * @brief A tuple of N integers.
 * @tparam N - Number of elements in this tuple.
 */
template <U64 N>
class Pos : public Tuple<N, I64, Pos<N>> {
public:
    using value_type = I64;

    using Tuple<N, I64, Pos>::Tuple;

    static Pos round(const Vec<N> &vec) {
        Pos result;
        for (U64 i = 0; i < N; ++i) {
            result[i] = static_cast<I64>(std::round(vec[i]));
        }
        return result;
    }

    /// Returns a new Pos with the result of element-wise clamping to the given grid.
    /// Elements are rounded up to the grid in each dimension.
    pure Pos grid_max(const Pos &grid) const {
        Pos result;
        for (U64 i = 0; i < N; i++) {
            result[i] = nvl::grid_max(this->indices_[i], grid[i]);
        }
        return result;
    }

    pure Pos grid_max(const I64 grid) const {
        Pos result;
        for (U64 i = 0; i < N; i++) {
            result[i] = nvl::grid_max(this->indices_[i], grid);
        }
        return result;
    }

    /// Returns a new Pos with the result of element-wise clamping to the given grid.
    /// Elements are rounded down to the grid in each dimension.
    pure Pos grid_min(const Pos &grid) const {
        Pos result;
        for (U64 i = 0; i < N; i++) {
            result[i] = nvl::grid_min(this->indices_[i], grid[i]);
        }
        return result;
    }

    pure Pos grid_min(const I64 grid) const {
        Pos result;
        for (U64 i = 0; i < N; i++) {
            result[i] = nvl::grid_min(this->indices_[i], grid);
        }
        return result;
    }

    pure I64 manhattan_dist(const Pos &rhs) const {
        I64 result = 0;
        for (U64 i = 0; i < N; ++i) {
            result += std::abs(this->indices_[i] - rhs.indices_[i]);
        }
        return result;
    }

    pure Vec<N> to_vec() const {
        Vec<N> result;
        for (U64 i = 0; i < N; ++i) {
            result[i] = static_cast<F64>(this->indices_[i]);
        }
        return result;
    }

private:
    friend std::hash<Pos>;
};

template <U64 N>
Pos<N> operator*(const I64 a, const Pos<N> &b) {
    return b * a;
}
template <U64 N>
Pos<N> operator/(const I64 a, const Pos<N> &b) {
    return Pos<N>::fill(a) / b;
}
template <U64 N>
Pos<N> operator+(const I64 a, const Pos<N> &b) {
    return b + a;
}
template <U64 N>
Pos<N> operator-(const I64 a, const Pos<N> &b) {
    return -b + a;
}

} // namespace nvl

template <U64 N>
struct std::hash<nvl::Pos<N>> {
    pure U64 operator()(const nvl::Pos<N> &a) const noexcept { return nvl::sip_hash(a.indices_); }
};