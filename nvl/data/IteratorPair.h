#pragma once

#include <algorithm>

#include "nvl/macros/Pure.h"

namespace nvl {

template <template <typename I> typename Concrete, typename IterType, typename Iterator, typename ConstIterator>
class IteratorPair {
public:
    using value_type = typename IterType::value_type;

    /// Creates an empty iterator pair.
    /// Only available if the underlying Iterator type is also default constructible.
    IteratorPair()
        requires std::is_default_constructible_v<IterType>
        : begin_(), end_() {}

    IteratorPair(IterType begin, IterType end) : begin_(begin), end_(end) {}

    virtual ~IteratorPair() = default;

    pure bool empty() const { return begin_ == end_; }

    pure Iterator begin() { return begin_; }
    pure ConstIterator begin() const { return begin_; }

    pure Iterator end() { return end_; }
    pure ConstIterator end() const { return end_; }

    pure bool operator==(const Concrete<IterType> &rhs) const { return begin_ == rhs.begin_; }
    pure bool operator!=(const Concrete<IterType> &rhs) const { return begin_ != rhs.begin_; }

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
    IterType begin_;
    IterType end_;
};

} // namespace nvl
