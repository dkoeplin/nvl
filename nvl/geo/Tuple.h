#pragma once

#include <cstdlib>
#include <iterator>
#include <sstream>

#include "nvl/data/Iterator.h"
#include "nvl/data/Maybe.h"
#include "nvl/data/SipHash.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/SIMD.h"
#include "nvl/math/Bitwise.h"
#include "nvl/math/Grid.h"
#include "nvl/reflect/ClassTag.h"

namespace nvl {

/**
 * @class Tuple
 * @brief A tuple with N elements of type T.
 * @tparam N - Number of elements in this tuple.
 * @tparam T - Type of elements.
 */
template <U64 N, typename T>
class Tuple {
public:
    using value_type = T;

    struct iterator final : AbstractIteratorCRTP<iterator, T> {
        class_tag(Tuple::iterator, AbstractIterator<T>);

        template <View Type>
        static Iterator<T, Type> begin(const Tuple &vec) {
            return make_iterator<iterator, Type>(vec, 0);
        }
        template <View Type>
        static Iterator<T, Type> end(const Tuple &vec) {
            return make_iterator<iterator, Type>(vec, N);
        }

        explicit iterator(const Tuple &vec, const U64 index) : index_(index), vec_(vec) {}

        void increment() override { ++index_; }

        pure const T *ptr() override { return &vec_.indices_[index_]; }

        pure bool operator==(const iterator &rhs) const override { return &vec_ == &rhs.vec_ && index_ == rhs.index_; }

        U64 index_ = 0;
        const Tuple &vec_;
    };

    /// Returns an instance of rank `N` where all elements are `value`.
    static constexpr Tuple fill(const T value) {
        Tuple result;
        for (U64 i = 0; i < N; ++i) {
            result[i] = value;
        }
        return result;
    }

    /// Returns an instance of rank `N` where all elements are zero except the one at `i`, which is `x`.
    static constexpr Tuple unit(const U64 i, const T x = 1) {
        ASSERT(i < N, "Index " << i << " is out of bounds [" << 0 << ", " << N << ")");
        Tuple result = fill(0);
        result[i] = x;
        return result;
    }

    /// An instance of rank `N` where all elements are zero.
    static const Tuple zero;

    /// An instance of rank `N` where all elements are one.
    static const Tuple ones;

    /// Returns a Tuple of rank `N` with uninitialized elements.
    explicit constexpr Tuple() = default;

    explicit constexpr Tuple(T a)
        requires(N == 1)
        : indices_{a} {}
    constexpr Tuple(T a, T b)
        requires(N == 2)
        : indices_{a, b} {}
    constexpr Tuple(T a, T b, T c)
        requires(N == 3)
        : indices_{a, b, c} {}
    constexpr Tuple(T a, T b, T c, T d)
        requires(N == 4)
        : indices_{a, b, c, d} {}
    constexpr Tuple(T a, T b, T c, T d, T e)
        requires(N == 5)
        : indices_{a, b, c, d, e} {}

    /// Implicitly converts this instance to a single element. Only valid for rank 1.
    explicit operator T() const {
        static_assert(N == 1, "Cannot convert Tuple of rank > 1 to integer.");
        return indices_[0];
    }

    pure MIterator<T> begin() { return iterator::template begin<View::kMutable>(*this); }
    pure MIterator<T> end() { return iterator::template end<View::kMutable>(*this); }
    pure Iterator<T> begin() const { return iterator::template begin<View::kImmutable>(*this); }
    pure Iterator<T> end() const { return iterator::template end<View::kImmutable>(*this); }

    /// Returns the rank of this instance.
    pure constexpr U64 rank() const { return N; }

    /// Returns the element at `i` or None if `i` is out of bounds.
    pure Maybe<T> get(const U64 i) const { return i < N ? Some(indices_[i]) : None; }

    /// Returns the element at `i` or `v` if `i` is out of bounds.
    pure T get_or(const U64 i, const T v) const { return i < N ? indices_[i] : v; }

    /// Returns the element at `i`.
    pure constexpr T operator[](const U64 i) const { return indices_[i]; }

    /// Returns a reference to the element at `i`.
    pure constexpr T &operator[](const U64 i) { return indices_[i]; }

    /// Returns a copy with the element at `i` changed to `v`.
    pure Tuple with(const U64 i, const T v) const {
        Tuple result = *this;
        result.indices_[i] = v;
        return result;
    }

    /// Returns a copy with every element negated.
    pure Tuple operator-() const {
        Tuple result = *this;
        simd for (T &x : result.indices_) { x = -x; }
        return result;
    }

    /// Returns a new instance with the result of element-wise multiplication.
    pure Tuple operator*(const Tuple &rhs) const {
        Tuple result;
        simd for (U64 i = 0; i < N; ++i) { result[i] = indices_[i] * rhs.indices_[i]; }
        return result;
    }

    /// Returns a new instance with the result of element-wise division.
    pure Tuple operator/(const Tuple &rhs) const {
        Tuple result;
        simd for (U64 i = 0; i < N; ++i) { result[i] = indices_[i] / rhs.indices_[i]; }
        return result;
    }

    /// Returns a new instance with the result of element-wise mod.
    pure Tuple operator%(const Tuple rhs) const
        requires std::is_integral_v<T>
    {
        Tuple result;
        simd for (U64 i = 0; i < N; ++i) { result[i] = indices_[i] % rhs.indices_[i]; }
        return result;
    }

    /// Returns a new instance with the result of element-wise addition.
    pure Tuple operator+(const Tuple &rhs) const {
        Tuple result;
        simd for (U64 i = 0; i < N; ++i) { result[i] = indices_[i] + rhs.indices_[i]; }
        return result;
    }

    /// Returns a new instance with the result of element-wise subtraction.
    pure Tuple operator-(const Tuple &rhs) const {
        Tuple result;
        simd for (U64 i = 0; i < N; ++i) { result[i] = indices_[i] - rhs.indices_[i]; }
        return result;
    }

    /// Returns a new instance with the result of element-wise multiplication.
    pure Tuple operator*(const T rhs) const {
        Tuple result = *this;
        simd for (T &x : result.indices_) { x *= rhs; }
        return result;
    }

    /// Returns a new instance with the result of element-wise division.
    pure Tuple operator/(const T rhs) const {
        Tuple result = *this;
        simd for (T &x : result.indices_) { x /= rhs; }
        return result;
    }

    /// Returns a new instance with the result of element-wise mod.
    pure Tuple operator%(const T rhs) const
        requires std::is_integral_v<T>
    {
        Tuple result = *this;
        simd for (T &x : result.indices_) { x %= rhs; }
        return result;
    }

    /// Returns a new instance with the result of element-wise addition.
    pure Tuple operator+(const T rhs) const {
        Tuple result = *this;
        simd for (T &x : result.indices_) { x += rhs; }
        return result;
    }

    /// Returns a new instance with the result of element-wise subtraction.
    pure Tuple operator-(const T rhs) const {
        Tuple result = *this;
        simd for (T &x : result.indices_) { x -= rhs; }
        return result;
    }

    /// Returns true if the two instances have identical elements.
    pure bool operator==(const Tuple &rhs) const {
        simd for (U64 i = 0; i < N; ++i) {
            if (indices_[i] != rhs.indices_[i]) {
                return false;
            }
        }
        return true;
    }

    /// Returns true if the two instances do not have identical elements.
    pure bool operator!=(const Tuple &rhs) const { return !(*this == rhs); }

    /// Returns true if every element is strictly less than the corresponding element in `rhs`.
    pure bool all_lt(const Tuple &rhs) const {
        simd for (U64 i = 0; i < N; ++i) {
            if (indices_[i] >= rhs.indices_[i]) {
                return false;
            }
        }
        return true;
    }

    /// Returns true if every element is less than or equal to the corresponding element in `rhs`.
    pure bool all_lte(const Tuple &rhs) const {
        simd for (U64 i = 0; i < N; ++i) {
            if (indices_[i] > rhs.indices_[i]) {
                return false;
            }
        }
        return true;
    }

    /// Returns true if every element is greater than the corresponding element in `rhs`.
    pure bool all_gt(const Tuple &rhs) const { return rhs.all_lt(*static_cast<const Tuple *>(this)); }

    /// Returns true if every element is greater than or equal to the corresponding element in `rhs`.
    pure bool all_gte(const Tuple &rhs) const { return rhs.all_lte(*static_cast<const Tuple *>(this)); }

    /// Returns a new instance with the result of element-wise clamping to the given grid.
    /// Elements are rounded up to the grid in each dimension.
    pure Tuple grid_max(const Tuple &grid) const {
        Tuple result;
        simd for (U64 i = 0; i < N; i++) { result[i] = nvl::grid_max(this->indices_[i], grid[i]); }
        return result;
    }

    pure Tuple grid_max(const T grid) const {
        Tuple result;
        simd for (U64 i = 0; i < N; i++) { result[i] = nvl::grid_max(this->indices_[i], grid); }
        return result;
    }

    /// Returns a new instance with the result of element-wise clamping to the given grid.
    /// Elements are rounded down to the grid in each dimension.
    pure Tuple grid_min(const Tuple &grid) const {
        Tuple result;
        simd for (U64 i = 0; i < N; i++) { result[i] = nvl::grid_min(this->indices_[i], grid[i]); }
        return result;
    }

    pure Tuple grid_min(const T grid) const {
        Tuple result;
        simd for (U64 i = 0; i < N; i++) { result[i] = nvl::grid_min(this->indices_[i], grid); }
        return result;
    }

    pure T manhattan_dist(const Tuple &rhs) const {
        T result = 0;
        simd for (U64 i = 0; i < N; ++i) { result += std::abs(this->indices_[i] - rhs.indices_[i]); }
        return result;
    }

    pure F64 dist(const Tuple &rhs) const {
        F64 result = 0;
        simd for (U64 i = 0; i < N; ++i) {
            const F64 diff = static_cast<F64>(indices_[i]) - rhs.indices_[i];
            result += diff * diff;
        }
        return std::sqrt(result);
    }

    pure F64 magnitude() const {
        F64 result = 0;
        simd for (U64 i = 0; i < N; ++i) { result += static_cast<F64>(indices_[i]) * indices_[i]; }
        return std::sqrt(result);
    }

    pure T product() const {
        T product = 1;
        for (const T x : indices_)
            product *= x;
        return product;
    }

    pure T sum() const {
        T sum = 0;
        for (const T x : indices_)
            sum += x;
        return sum;
    }

    pure T max() const { return *std::max_element(std::begin(indices_), std::end(indices_)); }
    pure T min() const { return *std::min_element(std::begin(indices_), std::end(indices_)); }

    pure Tuple strides() const {
        Tuple result;
        result[N - 1] = 1;
        for (I64 i = N - 2; i >= 0; --i) {
            result[i] = result[i + 1] * indices_[i + 1];
        }
        return result;
    }

    Tuple &operator*=(const Tuple &rhs) {
        simd for (U64 i = 0; i < N; ++i) { indices_[i] *= rhs.indices_[i]; }
        return *static_cast<Tuple *>(this);
    }
    Tuple &operator/=(const Tuple &rhs) {
        simd for (U64 i = 0; i < N; ++i) { indices_[i] /= rhs.indices_[i]; }
        return *static_cast<Tuple *>(this);
    }
    Tuple &operator%=(const Tuple &rhs) {
        simd for (U64 i = 0; i < N; ++i) { indices_[i] %= rhs.indices_[i]; }
        return *static_cast<Tuple *>(this);
    }
    Tuple &operator+=(const Tuple &rhs) {
        simd for (U64 i = 0; i < N; ++i) { indices_[i] += rhs.indices_[i]; }
        return *static_cast<Tuple *>(this);
    }
    Tuple &operator-=(const Tuple &rhs) {
        simd for (U64 i = 0; i < N; ++i) { indices_[i] -= rhs.indices_[i]; }
        return *static_cast<Tuple *>(this);
    }
    Tuple &operator*=(const T rhs) {
        simd for (T &x : indices_) { x *= rhs; }
        return *this;
    }
    Tuple &operator/=(const T rhs) {
        simd for (T &x : indices_) { x /= rhs; }
        return *this;
    }
    Tuple &operator%=(const T rhs) {
        simd for (T &x : indices_) { x %= rhs; }
        return *this;
    }
    Tuple &operator+=(const T rhs) {
        simd for (T &x : indices_) { x += rhs; }
        return *this;
    }
    Tuple &operator-=(const T rhs) {
        simd for (T &x : indices_) { x -= rhs; }
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

protected:
    friend struct std::hash<Tuple>;
    T indices_[N];
}; // namespace nvl

template <U64 N>
using Vec = Tuple<N, F64>;

template <U64 N>
using Pos = Tuple<N, I64>;

template <U64 N, typename T>
const Tuple<N, T> Tuple<N, T>::zero = Tuple::fill(0);

template <U64 N, typename T>
const Tuple<N, T> Tuple<N, T>::ones = Tuple::fill(1);

template <U64 N, typename T>
Tuple<N, T> min(const Tuple<N, T> &a, const Tuple<N, T> &b) {
    Tuple<N, T> result;
    simd for (U64 i = 0; i < N; ++i) { result[i] = std::min(a[i], b[i]); }
    return result;
}

template <U64 N, typename T>
Tuple<N, T> max(const Tuple<N, T> &a, const Tuple<N, T> &b) {
    Tuple<N, T> result;
    simd for (U64 i = 0; i < N; ++i) { result[i] = std::max(a[i], b[i]); }
    return result;
}

template <U64 N, typename C, typename T>
Tuple<N, T> operator*(const C a, const Tuple<N, T> &b) {
    return b * a;
}
template <U64 N, typename C, typename T>
Tuple<N, T> operator/(const C a, const Tuple<N, T> &b) {
    return Tuple<N, T>::fill(a) / b;
}
template <U64 N, typename C, typename T>
Tuple<N, T> operator+(const C a, const Tuple<N, T> &b) {
    return b + a;
}
template <U64 N, typename C, typename T>
Tuple<N, T> operator-(const C a, const Tuple<N, T> &b) {
    return -b + a;
}

template <U64 N, typename V>
Tuple<N, V> abs(const Tuple<N, V> &tuple) {
    Tuple<N, V> result;
    simd for (U64 i = 0; i < N; ++i) { result[i] = std::abs(tuple[i]); }
    return result;
}

template <U64 N>
Tuple<N, F64> real(const Tuple<N, I64> &tuple) {
    Tuple<N, F64> result;
    simd for (U64 i = 0; i < N; ++i) { result[i] = static_cast<F64>(tuple[i]); }
    return result;
}

template <U64 N>
Tuple<N, I64> round(const Tuple<N, F64> &tuple) {
    Tuple<N, I64> result;
    simd for (U64 i = 0; i < N; ++i) { result[i] = static_cast<I64>(std::round(tuple[i])); }
    return result;
}

template <U64 N>
Tuple<N, I64> floor(const Tuple<N, F64> &tuple) {
    Tuple<N, I64> result;
    simd for (U64 i = 0; i < N; ++i) { result[i] = static_cast<I64>(std::floor(tuple[i])); }
    return result;
}

template <U64 N>
Tuple<N, I64> ceil(const Tuple<N, F64> &tuple) {
    Tuple<N, I64> result;
    simd for (U64 i = 0; i < N; ++i) { result[i] = static_cast<I64>(std::ceil(tuple[i])); }
    return result;
}

template <U64 N, typename T>
std::ostream &operator<<(std::ostream &os, const Tuple<N, T> &a) {
    return os << a.to_string();
}

} // namespace nvl

template <U64 N, typename T>
struct std::hash<nvl::Tuple<N, T>> {
    pure U64 operator()(const nvl::Tuple<N, T> &a) const noexcept { return nvl::sip_hash(a.indices_); }
};
