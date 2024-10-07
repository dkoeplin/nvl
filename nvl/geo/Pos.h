#pragma once

#include <sstream>
#include <string>

#include "nvl/data/Iterator.h"
#include "nvl/data/Maybe.h"
#include "nvl/data/SipHash.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/Assert.h"
#include "nvl/macros/Pure.h"
#include "nvl/math/Grid.h"
#include "nvl/reflect/Casting.h"

namespace nvl {

template <U64 N>
class Pos {
public:
    using value_type = I64;

    struct iterator final : AbstractIteratorCRTP<iterator, I64> {
        class_tag(Pos<N>::iterator, AbstractIterator<I64>);

        template <View Type>
        static Iterator<I64, Type> begin(const Pos &pos) {
            return make_iterator<iterator, Type>(pos, 0);
        }
        template <View Type>
        static Iterator<I64, Type> end(const Pos &pos) {
            return make_iterator<iterator, Type>(pos, N);
        }

        explicit iterator(const Pos &pos, const U64 index) : index_(index), pos_(pos) {}

        void increment() override { ++index_; }

        pure const I64 *ptr() override { return &pos_.indices_[index_]; }

        pure bool operator==(const iterator &rhs) const override { return pos_ == rhs.pos_ && index_ == rhs.index_; }

        U64 index_ = 0;
        const Pos &pos_;
    };

    /// Returns a Pos of rank `N` where all elements are `value`.
    static constexpr Pos fill(const I64 value) {
        Pos result;
        for (U64 i = 0; i < N; ++i) {
            result[i] = value;
        }
        return result;
    }

    /// Returns a Pos of rank `N` where all elements are zero except the one at `i`, which is `x`.
    static constexpr Pos unit(const U64 i, const I64 x = 1) {
        ASSERT(i < N, "Index " << i << " is out of bounds [" << 0 << ", " << N << ")");
        Pos result = fill(0);
        result[i] = x;
        return result;
    }

    /// Returns a Pos of rank `N` where all elements are zero.
    static const Pos zero;

    /// Returns a Pos of rank `N` where all elements are one.
    static const Pos ones;

    /// Returns a Pos of rank `N` with uninitialized elements.
    explicit constexpr Pos() = default;

    explicit constexpr Pos(I64 a)
        requires(N == 1)
        : indices_{a} {}
    constexpr Pos(I64 a, I64 b)
        requires(N == 2)
        : indices_{a, b} {}
    constexpr Pos(I64 a, I64 b, I64 c)
        requires(N == 3)
        : indices_{a, b, c} {}
    constexpr Pos(I64 a, I64 b, I64 c, I64 d)
        requires(N == 4)
        : indices_{a, b, c, d} {}
    constexpr Pos(I64 a, I64 b, I64 c, I64 d, I64 e)
        requires(N == 5)
        : indices_{a, b, c, d, e} {}

    /// Implicitly converts this Pos to an integer. Only valid for rank 1.
    explicit operator I64() const {
        static_assert(N == 1, "Cannot convert Pos of rank > 1 to integer.");
        return indices_[0];
    }

    pure MIterator<I64> begin() { return iterator::template begin<View::kMutable>(*this); }
    pure MIterator<I64> end() { return iterator::template end<View::kMutable>(*this); }
    pure Iterator<I64> begin() const { return iterator::template begin<View::kImmutable>(*this); }
    pure Iterator<I64> end() const { return iterator::template end<View::kImmutable>(*this); }

    /// Returns the rank of this Pos.
    pure constexpr U64 rank() const { return N; }

    /// Returns the element at `i` or None if `i` is out of bounds.
    pure Maybe<I64> get(const U64 i) const { return i < N ? Some(indices_[i]) : None; }

    /// Returns the element at `i` or `v` if `i` is out of bounds.
    pure I64 get_or(const U64 i, const I64 v) const { return i < N ? indices_[i] : v; }

    /// Returns the element at `i`, asserting that `i` is within bounds.
    pure constexpr I64 operator[](const U64 i) const {
        ASSERT(i < N, "Index " << i << " is out of bounds [" << 0 << ", " << N << ")");
        return indices_[i];
    }
    /// Returns a reference to the element at `i`, asserting that `i` is within bounds.
    pure constexpr I64 &operator[](const U64 i) {
        ASSERT(i < N, "Index " << i << " is out of bounds [" << 0 << ", " << N << ")");
        return indices_[i];
    }

    /// Returns a copy of this Pos with the element at `i` changed to `v`.
    pure Pos with(const U64 i, I64 v) const {
        ASSERT(i < N, "Index " << i << " is out of bounds [" << 0 << ", " << N << ")");
        Pos result = *this;
        result.indices_[i] = v;
        return result;
    }

    /// Returns a copy of this Pos with every element negated.
    pure Pos operator-() const {
        Pos result = *this;
        for (I64 &x : result.indices_) {
            x = -x;
        }
        return result;
    }

    /// Returns a new Pos with the result of element-wise multiplication.
    pure Pos operator*(const Pos &rhs) const {
        Pos result;
        for (U64 i = 0; i < N; ++i) {
            result[i] = indices_[i] * rhs.indices_[i];
        }
        return result;
    }

    /// Returns a new Pos with the result of element-wise division.
    pure Pos operator/(const Pos &rhs) const {
        Pos result;
        for (U64 i = 0; i < N; ++i) {
            result[i] = indices_[i] / rhs.indices_[i];
        }
        return result;
    }

    /// Returns a new Pos with the result of element-wise addition.
    pure Pos operator+(const Pos &rhs) const {
        Pos result;
        for (U64 i = 0; i < N; ++i) {
            result[i] = indices_[i] + rhs.indices_[i];
        }
        return result;
    }

    /// Returns a new Pos with the result of element-wise subtraction.
    pure Pos operator-(const Pos &rhs) const {
        Pos result;
        for (U64 i = 0; i < N; ++i) {
            result[i] = indices_[i] - rhs.indices_[i];
        }
        return result;
    }

    /// Returns a new Pos with the result of element-wise multiplication.
    pure Pos operator*(const I64 rhs) const {
        Pos result = *this;
        for (I64 &x : result.indices_) {
            x *= rhs;
        }
        return result;
    }

    /// Returns a new Pos with the result of element-wise division.
    pure Pos operator/(const I64 rhs) const {
        Pos result = *this;
        for (I64 &x : result.indices_) {
            x /= rhs;
        }
        return result;
    }

    /// Returns a new Pos with the result of element-wise addition.
    pure Pos operator+(const I64 rhs) const {
        Pos result = *this;
        for (I64 &x : result.indices_) {
            x += rhs;
        }
        return result;
    }

    /// Returns a new Pos with the result of element-wise subtraction.
    pure Pos operator-(const I64 rhs) const {
        Pos result = *this;
        for (I64 &x : result.indices_) {
            x -= rhs;
        }
        return result;
    }

    /// Returns a new Pos with the result of element-wise clamping to the given grid.
    /// Elements are rounded up to the grid in each dimension.
    pure Pos grid_max(const Pos &grid) const {
        Pos result;
        for (U64 i = 0; i < N; i++) {
            result[i] = nvl::grid_max(indices_[i], grid[i]);
        }
        return result;
    }

    pure Pos grid_max(const I64 grid) const {
        Pos result;
        for (U64 i = 0; i < N; i++) {
            result[i] = nvl::grid_max(indices_[i], grid);
        }
        return result;
    }

    /// Returns a new Pos with the result of element-wise clamping to the given grid.
    /// Elements are rounded down to the grid in each dimension.
    pure Pos grid_min(const Pos &grid) const {
        Pos result;
        for (U64 i = 0; i < N; i++) {
            result[i] = nvl::grid_min(indices_[i], grid[i]);
        }
        return result;
    }

    pure Pos grid_min(const I64 grid) const {
        Pos result;
        for (U64 i = 0; i < N; i++) {
            result[i] = nvl::grid_min(indices_[i], grid);
        }
        return result;
    }

    /// Returns true if the two Pos instances have identical elements.
    pure bool operator==(const Pos &rhs) const {
        for (U64 i = 0; i < N; ++i) {
            if (indices_[i] != rhs.indices_[i]) {
                return false;
            }
        }
        return true;
    }

    /// Returns true if the two Pos instances do not have identical elements.
    pure bool operator!=(const Pos &rhs) const { return !(*this == rhs); }

    /// Returns true if every element is strictly less than the corresponding element in `rhs`.
    pure bool all_lt(const Pos &rhs) const {
        for (U64 i = 0; i < N; ++i) {
            if (indices_[i] >= rhs.indices_[i]) {
                return false;
            }
        }
        return true;
    }

    /// Returns true if every element is less than or equal to the corresponding element in `rhs`.
    pure bool all_lte(const Pos &rhs) const {
        for (U64 i = 0; i < N; ++i) {
            if (indices_[i] > rhs.indices_[i]) {
                return false;
            }
        }
        return true;
    }

    /// Returns true if every element is greater than the corresponding element in `rhs`.
    pure bool all_gt(const Pos &rhs) const { return rhs.all_lt(*this); }

    /// Returns true if every element is greater than or equal to the corresponding element in `rhs`.
    pure bool all_gte(const Pos &rhs) const { return rhs.all_lte(*this); }

    pure I64 manhattan_dist(const Pos &rhs) const {
        I64 result = 0;
        for (U64 i = 0; i < N; ++i) {
            result += std::abs(indices_[i] - rhs.indices_[i]);
        }
        return result;
    }

    pure F64 dist(const Pos &rhs) const {
        F64 result = 0;
        for (U64 i = 0; i < N; ++i) {
            const F64 diff = indices_[i] - rhs.indices_[i];
            result += diff * diff;
        }
        return std::sqrt(result);
    }

    pure F64 magnitude() const {
        F64 result = 0;
        for (U64 i = 0; i < N; ++i) {
            result += static_cast<F64>(indices_[i]) * indices_[i];
        }
        return std::sqrt(result);
    }

    pure I64 product() const {
        I64 product = 1;
        for (const I64 x : indices_)
            product *= x;
        return product;
    }

    pure I64 sum() const {
        I64 sum = 0;
        for (const I64 x : indices_)
            sum += x;
        return sum;
    }

    pure Pos strides() const {
        Pos result;
        result[N - 1] = 1;
        for (I64 i = N - 2; i >= 0; --i) {
            result[i] = result[i + 1] * indices_[i + 1];
        }
        return result;
    }

    Pos &operator*=(const Pos &rhs) {
        for (U64 i = 0; i < N; ++i) {
            indices_[i] *= rhs.indices_[i];
        }
        return *this;
    }
    Pos &operator/=(const Pos &rhs) {
        for (U64 i = 0; i < N; ++i) {
            indices_[i] /= rhs.indices_[i];
        }
        return *this;
    }
    Pos &operator+=(const Pos &rhs) {
        for (U64 i = 0; i < N; ++i) {
            indices_[i] += rhs.indices_[i];
        }
        return *this;
    }
    Pos &operator-=(const Pos &rhs) {
        for (U64 i = 0; i < N; ++i) {
            indices_[i] -= rhs.indices_[i];
        }
        return *this;
    }
    Pos &operator*=(const I64 rhs) {
        for (I64 &x : indices_) {
            x *= rhs;
        }
        return *this;
    }
    Pos &operator/=(const I64 rhs) {
        for (I64 &x : indices_) {
            x /= rhs;
        }
        return *this;
    }
    Pos &operator+=(const I64 rhs) {
        for (I64 &x : indices_) {
            x += rhs;
        }
        return *this;
    }
    Pos &operator-=(const I64 rhs) {
        for (I64 &x : indices_) {
            x -= rhs;
        }
        return *this;
    }

    pure std::string to_string() const {
        std::stringstream ss;
        ss << "{" << indices_[0];
        for (U64 i = 1; i < N; ++i) {
            ss << ", " << indices_[i];
        }
        ss << "}";
        return ss.str();
    }

private:
    friend struct std::hash<Pos>;
    I64 indices_[N];
};

template <U64 N>
const Pos<N> Pos<N>::zero = Pos::fill(0);

template <U64 N>
const Pos<N> Pos<N>::ones = Pos::fill(1);

template <U64 N>
Pos<N> min(const Pos<N> &a, const Pos<N> &b) {
    Pos<N> result;
    for (U64 i = 0; i < N; ++i) {
        result[i] = std::min(a[i], b[i]);
    }
    return result;
}

template <U64 N>
Pos<N> max(const Pos<N> &a, const Pos<N> &b) {
    Pos<N> result;
    for (U64 i = 0; i < N; ++i) {
        result[i] = std::max(a[i], b[i]);
    }
    return result;
}

template <U64 N>
Pos<N> operator*(I64 a, const Pos<N> &b) {
    return b * a;
}
template <U64 N>
Pos<N> operator/(I64 a, const Pos<N> &b) {
    return Pos<N>::fill(a) / b;
}
template <U64 N>
Pos<N> operator+(I64 a, const Pos<N> &b) {
    return b + a;
}
template <U64 N>
Pos<N> operator-(I64 a, const Pos<N> &b) {
    return -b + a;
}

template <U64 N>
std::ostream &operator<<(std::ostream &os, const Pos<N> &a) {
    return os << a.to_string();
}

} // namespace nvl

template <U64 N>
struct std::hash<nvl::Pos<N>> {
    pure U64 operator()(const nvl::Pos<N> &a) const noexcept { return nvl::sip_hash(a.indices_); }
};
