#pragma once

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
 *   static Iterator Iterator::empty();
 *   static Iterator Iterator::begin(args...);
 *   static Iterator Iterator::end(args...);
 * };
 */
template <typename Iterator>
class Range {
  public:
    Range() : begin_(Iterator::empty()), end_(Iterator::empty()), empty_(true) {}

    template <typename... Args>
    explicit Range(Args &&...args)
        : begin_(Iterator::begin(std::forward<Args>(args)...)), end_(Iterator::end(std::forward<Args>(args)...)),
          empty_(false) {}

    explicit Range(Iterator begin, Iterator end) : begin_(begin), end_(end), empty_(false) {}

    pure Iterator begin() const { return empty_ ? end_ : begin_; }
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

  private:
    Iterator begin_;
    Iterator end_;
    bool empty_;
};

} // namespace nox
