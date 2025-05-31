#pragma once

#include <iostream>

#include "nvl/data/Iterator.h"
#include "nvl/data/Once.h"
#include "nvl/macros/Pure.h"

namespace nvl {

/**
 * @class Range
 * @brief A pair of iterators which can be iterated over any number of times.
 *
 * Iterating over a Range causes the `begin` iterator to be copied to allow repeated iteration.
 *
 * @tparam Value - The value type being iterated over.
 * @tparam Type - The type of view this range provides over the underlying elements (mutable or immutable).
 */
template <typename Value, View Type = View::kImmutable>
class Range final {
public:
    using value_type = Value;

    /// Creates an empty iterator pair.
    /// Only available if the underlying Iterator type is also default constructible.
    Range() : begin_(), end_() {}

    Range(Iterator<Value, Type> begin, Iterator<Value, Type> end) : begin_(begin), end_(end) {}

    implicit Range(const Range<Value, View::kMutable> &rhs)
        requires(Type == View::kImmutable)
        : Range(*(const Range<Value> *)(&rhs)) {}

    /// Returns a copy of this Range as a single iteration Once range.
    pure Once<Value, Type> once() const { return Once<Value, Type>(begin_.copy(), end_); }

    pure bool empty() const { return begin_ == end_; }

    /// Returns the number of elements in this range. Note that this is O(N)!
    pure U64 size() const {
        U64 size = 0;
        const auto end = this->end();
        for (auto iter = begin(); iter != end; ++iter) {
            ++size;
        }
        return size;
    }

    pure Iterator<Value, Type> begin() const { return begin_.copy(); }
    pure Iterator<Value, Type> end() const { return end_; }

    pure bool operator==(const Range &rhs) const { return begin_ == rhs.begin_; }
    pure bool operator!=(const Range &rhs) const { return begin_ != rhs.begin_; }

    /// Returns true if all values in this range meet the given condition.
    template <typename Cond>
    pure bool all(const Cond &cond) const {
        return std::all_of(begin(), end(), cond);
    }

    /// Returns true if at least one value in this range meet the given condition.
    template <typename Cond>
    pure bool exists(const Cond &cond) const {
        return std::any_of(begin(), end(), cond);
    }

protected:
    Iterator<Value, Type> begin_;
    Iterator<Value, Type> end_;
};

template <typename Value>
using MRange = Range<Value, View::kMutable>;

/**
 * Returns a new Range instance with `begin` and `end` iterators.
 *
 * Requires that the Iterator type has at least the following:
 * class Iterator {
 *   using value_type = ...;
 *   static Iterator::begin(args...);
 *   static Iterator::end(args...);
 * }
 */
template <typename IterType, View Type = View::kImmutable, typename... Args>
Range<typename IterType::value_type, Type> make_range(Args &&...args) {
    using Value = typename IterType::value_type;
    Iterator<Value, Type> i0 = IterType::template begin<Type>(std::forward<Args>(args)...);
    Iterator<Value, Type> i1 = IterType::template end<Type>(std::forward<Args>(args)...);
    return Range<Value, Type>(i0, i1);
}

template <typename IterType, typename... Args>
MRange<typename IterType::value_type> make_mrange(Args &&...args) {
    using Value = typename IterType::value_type;
    Iterator<Value, View::kMutable> i0 = IterType::template begin<View::kMutable>(std::forward<Args>(args)...);
    Iterator<Value, View::kMutable> i1 = IterType::template end<View::kMutable>(std::forward<Args>(args)...);
    return Range<Value, View::kMutable>(i0, i1);
}

template <typename Value>
std::ostream &operator<<(std::ostream &os, const Range<Value> &range) {
    os << "{";
    auto iter = range.begin();
    const auto end = range.end();
    if (iter != end) {
        os << *iter;
        ++iter;
    }
    for (; iter != end; ++iter) {
        os << ", " << *iter;
    }
    return os << "}";
}

} // namespace nvl
