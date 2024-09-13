#pragma once

#include "nox/data/List.h"
#include "nox/data/Once.h"
#include "nox/macros/Pure.h"

namespace nox {

/**
 * @class Range
 * @brief A pair of iterators.
 * Iterating over a Range causes the `begin` iterator to be copied to allow repeated iteration.
 *
 * Requires that the Iterator type has at least the following:
 * class Iterator {
 *   using value_type = ...;
 *   Iterator(); // Default constructor
 *   static Iterator Iterator::begin(args...);
 *   static Iterator Iterator::end(args...);
 * };
 */
template <typename Iterator>
class Range {
  public:
    using value_type = typename Iterator::value_type;

    /// Creates an empty Range.
    /// Only available if the underlying Iterator type is also default constructible.
    Range()
        requires std::is_default_constructible_v<Iterator>
        : begin_(), end_() {}

    template <typename... Args>
    explicit Range(Args &&...args)
        : begin_(Iterator::begin(std::forward<Args>(args)...)), end_(Iterator::end(std::forward<Args>(args)...)) {}

    Range(Iterator begin, Iterator end) : begin_(begin), end_(end) {}

    /// Returns a _copy_ of the `begin` Iterator.
    pure Iterator begin() const { return begin_; }

    /// Returns a _copy_ of the `end` Iterator.
    pure Iterator end() const { return end_; }

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

    /// Returns a copy of this Range as a single iteration Once range.
    pure Once<Iterator> once() const { return Once<Iterator>(begin_, end_); }

    /// Returns a copy of the elements in this Range in a List.
    pure List<value_type> list() const { return List<value_type>(begin(), end()); }

  private:
    Iterator begin_;
    Iterator end_;
};

template <typename Iterator>
std::ostream &operator<<(std::ostream &os, const Range<Iterator> &range) {
    os << "{";
    auto iter = range.begin();
    if (iter != range.end()) {
        os << *iter;
    }
    for (; iter != range.end(); ++iter) {
        os << ", " << *iter;
    }
    return os << "}";
}

} // namespace nox
