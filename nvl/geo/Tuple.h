#pragma once

#include <sstream>

#include "nvl/data/Iterator.h"
#include "nvl/data/Maybe.h"
#include "nvl/data/SipHash.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/Assert.h"
#include "nvl/reflect/ClassTag.h"

namespace nvl {

/**
 * @class Tuple
 * @brief A tuple with N elements of type T.
 * @tparam N - Number of elements in this tuple.
 * @tparam T - Type of elements.
 */
template <U64 N, typename T, typename Concrete>
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

    /// Returns a Tuple of rank `N` where all elements are `value`.
    static constexpr Concrete fill(const T value) {
        Concrete result;
        for (U64 i = 0; i < N; ++i) {
            result[i] = value;
        }
        return result;
    }

    /// Returns a Tuple of rank `N` where all elements are zero except the one at `i`, which is `x`.
    static constexpr Concrete unit(const U64 i, const T x = 1) {
        ASSERT(i < N, "Index " << i << " is out of bounds [" << 0 << ", " << N << ")");
        Concrete result = fill(0);
        result[i] = x;
        return result;
    }

    /// Returns a Tuple of rank `N` where all elements are zero.
    static const Concrete zero;

    /// Returns a Tuple of rank `N` where all elements are one.
    static const Concrete ones;

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

    /// Implicitly converts this Tuple to a single element. Only valid for rank 1.
    explicit operator T() const {
        static_assert(N == 1, "Cannot convert Tuple of rank > 1 to integer.");
        return indices_[0];
    }

    pure MIterator<T> begin() { return iterator::template begin<View::kMutable>(*this); }
    pure MIterator<T> end() { return iterator::template end<View::kMutable>(*this); }
    pure Iterator<T> begin() const { return iterator::template begin<View::kImmutable>(*this); }
    pure Iterator<T> end() const { return iterator::template end<View::kImmutable>(*this); }

    /// Returns the rank of this Tuple.
    pure constexpr U64 rank() const { return N; }

    /// Returns the element at `i` or None if `i` is out of bounds.
    pure Maybe<T> get(const U64 i) const { return i < N ? Some(indices_[i]) : None; }

    /// Returns the element at `i` or `v` if `i` is out of bounds.
    pure T get_or(const U64 i, const T v) const { return i < N ? indices_[i] : v; }

    /// Returns the element at `i`, asserting that `i` is within bounds.
    pure constexpr I64 operator[](const U64 i) const {
        ASSERT(i < N, "Index " << i << " is out of bounds [" << 0 << ", " << N << ")");
        return indices_[i];
    }
    /// Returns a reference to the element at `i`, asserting that `i` is within bounds.
    pure constexpr T &operator[](const U64 i) {
        ASSERT(i < N, "Index " << i << " is out of bounds [" << 0 << ", " << N << ")");
        return indices_[i];
    }

    /// Returns a copy of this Tuple with the element at `i` changed to `v`.
    pure Concrete with(const U64 i, const T v) const {
        ASSERT(i < N, "Index " << i << " is out of bounds [" << 0 << ", " << N << ")");
        Concrete result = *static_cast<const Concrete *>(this);
        result.indices_[i] = v;
        return result;
    }

    /// Returns a copy of this Tuple with every element negated.
    pure Concrete operator-() const {
        Concrete result = *static_cast<const Concrete *>(this);
        for (I64 &x : result.indices_) {
            x = -x;
        }
        return result;
    }

    /// Returns a new Tuple with the result of element-wise multiplication.
    pure Concrete operator*(const Concrete &rhs) const {
        Concrete result;
        for (U64 i = 0; i < N; ++i) {
            result[i] = indices_[i] * rhs.indices_[i];
        }
        return result;
    }

    /// Returns a new Tuple with the result of element-wise division.
    pure Concrete operator/(const Concrete &rhs) const {
        Concrete result;
        for (U64 i = 0; i < N; ++i) {
            result[i] = indices_[i] / rhs.indices_[i];
        }
        return result;
    }

    /// Returns a new Tuple with the result of element-wise addition.
    pure Concrete operator+(const Concrete &rhs) const {
        Concrete result;
        for (U64 i = 0; i < N; ++i) {
            result[i] = indices_[i] + rhs.indices_[i];
        }
        return result;
    }

    /// Returns a new Tuple with the result of element-wise subtraction.
    pure Concrete operator-(const Concrete &rhs) const {
        Concrete result;
        for (U64 i = 0; i < N; ++i) {
            result[i] = indices_[i] - rhs.indices_[i];
        }
        return result;
    }

    /// Returns a new Tuple with the result of element-wise multiplication.
    pure Concrete operator*(const T rhs) const {
        Concrete result = *static_cast<const Concrete *>(this);
        for (I64 &x : result.indices_) {
            x *= rhs;
        }
        return result;
    }

    /// Returns a new Tuple with the result of element-wise division.
    pure Concrete operator/(const T rhs) const {
        Concrete result = *static_cast<const Concrete *>(this);
        for (I64 &x : result.indices_) {
            x /= rhs;
        }
        return result;
    }

    /// Returns a new Tuple with the result of element-wise addition.
    pure Concrete operator+(const T rhs) const {
        Concrete result = *static_cast<const Concrete *>(this);
        for (I64 &x : result.indices_) {
            x += rhs;
        }
        return result;
    }

    /// Returns a new Tuple with the result of element-wise subtraction.
    pure Concrete operator-(const T rhs) const {
        Concrete result = *static_cast<const Concrete *>(this);
        for (I64 &x : result.indices_) {
            x -= rhs;
        }
        return result;
    }

    /// Returns true if the two Tuple instances have identical elements.
    pure bool operator==(const Concrete &rhs) const {
        for (U64 i = 0; i < N; ++i) {
            if (indices_[i] != rhs.indices_[i]) {
                return false;
            }
        }
        return true;
    }

    /// Returns true if the two Tuple instances do not have identical elements.
    pure bool operator!=(const Concrete &rhs) const { return !(*this == rhs); }

    /// Returns true if every element is strictly less than the corresponding element in `rhs`.
    pure bool all_lt(const Concrete &rhs) const {
        for (U64 i = 0; i < N; ++i) {
            if (indices_[i] >= rhs.indices_[i]) {
                return false;
            }
        }
        return true;
    }

    /// Returns true if every element is less than or equal to the corresponding element in `rhs`.
    pure bool all_lte(const Concrete &rhs) const {
        for (U64 i = 0; i < N; ++i) {
            if (indices_[i] > rhs.indices_[i]) {
                return false;
            }
        }
        return true;
    }

    /// Returns true if every element is greater than the corresponding element in `rhs`.
    pure bool all_gt(const Concrete &rhs) const { return rhs.all_lt(*static_cast<const Concrete *>(this)); }

    /// Returns true if every element is greater than or equal to the corresponding element in `rhs`.
    pure bool all_gte(const Concrete &rhs) const { return rhs.all_lte(*static_cast<const Concrete *>(this)); }

    pure F64 dist(const Concrete &rhs) const {
        F64 result = 0;
        for (U64 i = 0; i < N; ++i) {
            const F64 diff = static_cast<F64>(indices_[i]) - rhs.indices_[i];
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

    pure Concrete strides() const {
        Concrete result;
        result[N - 1] = 1;
        for (I64 i = N - 2; i >= 0; --i) {
            result[i] = result[i + 1] * indices_[i + 1];
        }
        return result;
    }

    Concrete &operator*=(const Concrete &rhs) {
        for (U64 i = 0; i < N; ++i) {
            indices_[i] *= rhs.indices_[i];
        }
        return *static_cast<Concrete *>(this);
    }
    Concrete &operator/=(const Concrete &rhs) {
        for (U64 i = 0; i < N; ++i) {
            indices_[i] /= rhs.indices_[i];
        }
        return *static_cast<Concrete *>(this);
    }
    Concrete &operator+=(const Concrete &rhs) {
        for (U64 i = 0; i < N; ++i) {
            indices_[i] += rhs.indices_[i];
        }
        return *static_cast<Concrete *>(this);
    }
    Concrete &operator-=(const Concrete &rhs) {
        for (U64 i = 0; i < N; ++i) {
            indices_[i] -= rhs.indices_[i];
        }
        return *static_cast<Concrete *>(this);
    }
    Concrete &operator*=(const T rhs) {
        for (T &x : indices_) {
            x *= rhs;
        }
        return *static_cast<Concrete *>(this);
    }
    Concrete &operator/=(const T rhs) {
        for (T &x : indices_) {
            x /= rhs;
        }
        return *static_cast<Concrete *>(this);
    }
    Concrete &operator+=(const T rhs) {
        for (T &x : indices_) {
            x += rhs;
        }
        return *static_cast<Concrete *>(this);
    }
    Concrete &operator-=(const T rhs) {
        for (T &x : indices_) {
            x -= rhs;
        }
        return *static_cast<Concrete *>(this);
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
};

template <U64 N, typename T, typename Concrete>
const Concrete Tuple<N, T, Concrete>::zero = Tuple::fill(0);

template <U64 N, typename T, typename Concrete>
const Concrete Tuple<N, T, Concrete>::ones = Tuple::fill(1);

template <U64 N, typename T, typename Concrete>
Concrete min(const Tuple<N, T, Concrete> &a, const Tuple<N, T, Concrete> &b) {
    Concrete result;
    for (U64 i = 0; i < N; ++i) {
        result[i] = std::min(a[i], b[i]);
    }
    return result;
}

template <U64 N, typename T, typename Concrete>
Concrete max(const Tuple<N, T, Concrete> &a, const Tuple<N, T, Concrete> &b) {
    Concrete result;
    for (U64 i = 0; i < N; ++i) {
        result[i] = std::max(a[i], b[i]);
    }
    return result;
}

template <U64 N, typename T, typename Concrete>
Concrete operator*(const T a, const Tuple<N, T, Concrete> &b) {
    return b * a;
}
template <U64 N, typename T, typename Concrete>
Concrete operator/(const T a, const Tuple<N, T, Concrete> &b) {
    return Tuple<N, T, Concrete>::fill(a) / b;
}
template <U64 N, typename T, typename Concrete>
Concrete operator+(const T a, const Tuple<N, T, Concrete> &b) {
    return b + a;
}
template <U64 N, typename T, typename Concrete>
Concrete operator-(const T a, const Tuple<N, T, Concrete> &b) {
    return -b + a;
}

template <U64 N, typename T, typename Concrete>
std::ostream &operator<<(std::ostream &os, const Tuple<N, T, Concrete> &a) {
    return os << a.to_string();
}

} // namespace nvl

template <U64 N, typename T, typename Concrete>
struct std::hash<nvl::Tuple<N, T, Concrete>> {
    pure U64 operator()(const nvl::Tuple<N, T, Concrete> &a) const noexcept { return nvl::sip_hash(a.indices_); }
};
