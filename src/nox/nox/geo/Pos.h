#pragma once

#include <sstream>
#include <string>

#include "nox/data/Maybe.h"
#include "nox/data/SipHash.h"
#include "nox/macros/Aliases.h"
#include "nox/macros/Assert.h"
#include "nox/macros/Pure.h"
#include "nox/math/Clamping.h"

namespace nox {

template <U64 N> class Pos {
  public:
    using value_type = I64;

    class iterator {
      public:
        pure bool operator==(const iterator &rhs) const { return parent_ == rhs.parent_ && index_ == rhs.index_; }
        pure bool operator!=(const iterator &rhs) const { return !(*this == rhs); }

        I64 &operator->() const { return parent_.indices_[index_]; }
        I64 &operator*() const { return parent_.indices_[index_]; }
        iterator &operator++() {
            ++index_;
            return *this;
        }

      private:
        friend class Pos;
        static iterator begin(Pos &parent) { return iterator(parent, 0); }
        static iterator end(Pos &parent) { return iterator(parent, N); }
        explicit iterator(Pos &parent, U64 index) : parent_(parent), index_(index) {}
        U64 index_ = 0;
        Pos &parent_;
    };
    class const_iterator {
      public:
        pure bool operator==(const const_iterator &rhs) const { return parent_ == rhs.parent_ && index_ == rhs.index_; }
        pure bool operator!=(const const_iterator &rhs) const { return !(*this == rhs); }

        pure const I64 &operator->() const { return parent_.indices_[index_]; }
        pure const I64 &operator*() const { return parent_.indices_[index_]; }
        const_iterator &operator++() {
            ++index_;
            return *this;
        }

      private:
        friend class Pos;
        static const_iterator begin(const Pos &parent) { return const_iterator(parent, 0); }
        static const_iterator end(const Pos &parent) { return const_iterator(parent, N); }
        explicit const_iterator(const Pos &parent, U64 index) : parent_(parent), index_(index) {}

        U64 index_ = 0;
        const Pos &parent_;
    };

    /// Returns a Pos of rank `N` where all elements are `value`.
    static Pos fill(const I64 value) {
        Pos result;
        for (I64 &x : result) {
            x = value;
        }
        return result;
    }
    /// Returns a Pos of rank `N` where all elements are zero except the one at `i`, which is 1.
    static Pos unit(const I64 i) {
        ASSERT(i >= 0 && i < N, "Index " << i << " is out of bounds [" << 0 << ", " << N << ")");
        Pos result = fill(0);
        result[i] = 1;
        return result;
    }
    /// Returns a Pos of rank `N` where all elements are zero.
    static Pos zero() { return fill(0); }

    /// Returns a Pos of rank `N` with uninitialized elements.
    explicit constexpr Pos() = default;

    explicit constexpr Pos(I64 a) : indices_{a} {
        static_assert(N == 1, "Cannot construct Pos of rank > 1 from integer.");
    }
    constexpr Pos(I64 a, I64 b) : indices_{a, b} { static_assert(N == 2, "Expected N != 2 elements"); }
    constexpr Pos(I64 a, I64 b, I64 c) : indices_{a, b, c} { static_assert(N == 3, "Expected N != 3 elements"); }
    constexpr Pos(I64 a, I64 b, I64 c, I64 d) : indices_{a, b, c, d} {
        static_assert(N == 4, "Expected N != 4 elements");
    }
    constexpr Pos(I64 a, I64 b, I64 c, I64 d, I64 e) : indices_{a, b, c, d, e} {
        static_assert(N == 5, "Expected N != 5 elements");
    }

    /// Implicitly converts this Pos to an integer. Only valid for rank 1.
    explicit operator I64() const {
        static_assert(N == 1, "Cannot convert Pos of rank > 1 to integer.");
        return indices_[0];
    }

    iterator begin() { return iterator::begin(*this); }
    iterator end() { return iterator::end(*this); }
    pure const_iterator begin() const { return const_iterator::begin(*this); }
    pure const_iterator end() const { return const_iterator::end(*this); }

    /// Returns the rank of this Pos.
    pure constexpr U64 rank() const { return N; }

    /// Returns the element at `i` or None if `i` is out of bounds.
    pure Maybe<I64> get(const I64 i) const {
        if (i >= 0 && i < rank())
            return Some(indices_[i]);
        return None;
    }
    /// Returns the element at `i` or `v` if `i` is out of bounds.
    pure I64 get_or(const I64 i, const I64 v) const {
        if (i > 0 && i < rank()) {
            return indices_[i];
        }
        return v;
    }
    /// Returns the element at `i`, asserting that `i` is within bounds.
    pure I64 operator[](const I64 i) const {
        ASSERT(i >= 0 && i < N, "Index " << i << " is out of bounds [" << 0 << ", " << N << ")");
        return indices_[i];
    }
    /// Returns a reference to the element at `i`, asserting that `i` is within bounds.
    I64 &operator[](const I64 i) {
        ASSERT(i >= 0 && i < N, "Index " << i << " is out of bounds [" << 0 << ", " << N << ")");
        return indices_[i];
    }

    /// Returns a copy of this Pos with the element at `i` changed to `v`.
    pure Pos with(const I64 i, I64 v) const {
        ASSERT(i >= 0 && i < N, "Index " << i << " is out of bounds [" << 0 << ", " << N << ")");
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
    pure Pos clamp_up(const Pos &grid) const {
        Pos result;
        for (U64 i = 0; i < N; i++) {
            result[i] = nox::clamp_up(indices_[i], grid[i]);
        }
        return result;
    }

    pure Pos clamp_up(const I64 grid) const {
        Pos result;
        for (U64 i = 0; i < N; i++) {
            result[i] = nox::clamp_up(indices_[i], grid);
        }
        return result;
    }

    /// Returns a new Pos with the result of element-wise clamping to the given grid.
    /// Elements are rounded down to the grid in each dimension.
    pure Pos clamp_down(const Pos &grid) const {
        Pos result;
        for (U64 i = 0; i < N; i++) {
            result[i] = nox::clamp_down(indices_[i], grid[i]);
        }
        return result;
    }

    pure Pos clamp_down(const I64 grid) const {
        Pos result;
        for (U64 i = 0; i < N; i++) {
            result[i] = nox::clamp_down(indices_[i], grid);
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

    pure double dist(const Pos &rhs) const {
        double result = 0;
        for (U64 i = 0; i < N; ++i) {
            const double diff = indices_[i] - rhs.indices_[i];
            result += diff * diff;
        }
        return std::sqrt(result);
    }

    pure double magnitude() const {
        double result = 0;
        for (U64 i = 0; i < N; ++i) {
            result += static_cast<double>(indices_[i]) * indices_[i];
        }
        return std::sqrt(result);
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

template <U64 N> Pos<N> min(const Pos<N> &a, const Pos<N> &b) {
    Pos<N> result;
    for (U64 i = 0; i < N; ++i) {
        result[i] = std::min(a[i], b[i]);
    }
    return result;
}

template <U64 N> Pos<N> max(const Pos<N> &a, const Pos<N> &b) {
    Pos<N> result;
    for (U64 i = 0; i < N; ++i) {
        result[i] = std::max(a[i], b[i]);
    }
    return result;
}

template <U64 N> Pos<N> operator*(I64 a, const Pos<N> &b) { return b * a; }
template <U64 N> Pos<N> operator/(I64 a, const Pos<N> &b) { return Pos<N>::fill(a) / b; }
template <U64 N> Pos<N> operator+(I64 a, const Pos<N> &b) { return b + a; }
template <U64 N> Pos<N> operator-(I64 a, const Pos<N> &b) { return -b + a; }

template <U64 N> std::ostream &operator<<(std::ostream &os, const Pos<N> &a) { return os << a.to_string(); }

} // namespace nox

template <U64 N> struct std::hash<nox::Pos<N>> {
    pure U64 operator()(const nox::Pos<N> &a) const { return nox::sip_hash(a.indices_); }
};
