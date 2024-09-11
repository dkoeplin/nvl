#pragma once

#include <algorithm>

#include "nox/macros/Pure.h"

namespace nox {

/**
 * @class Once
 * @brief A pair of iterators which can be iterated over exactly once.
 * Iterating over a Once causes the `begin` iterator to be modified.
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
class Once {
  public:
    Once()
        requires std::is_default_constructible_v<Iterator>
        : begin_(), end_(), empty_(true) {}

    template <typename... Args>
    explicit Once(Args &&...args)
        : begin_(Iterator::begin(std::forward<Args>(args)...)), end_(Iterator::end(std::forward<Args>(args)...)),
          empty_(false) {}

    Once(Iterator begin, Iterator end) : begin_(begin), end_(end), empty_(false) {}

    pure bool has_next() const { return begin_ != end_; }

    pure Iterator &begin() { return empty_ ? end_ : begin_; }
    pure Iterator &end() { return end_; }

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
