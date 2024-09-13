#pragma once

#include <algorithm>

#include "nox/macros/Pure.h"

namespace nox {

template <typename T>
concept HasDereferenceOperator = requires(T a) { *a; };

template <typename T>
concept HasMemberAccessOperator = requires(T a) { a.operator->(); };

/**
 * @class Once
 * @brief A pair of iterators which can be iterated over exactly once.
 *
 * Most uses of Once are destructive, as they cause the `begin` iterator to be modified.
 *
 * Requires that the Iterator type has at least the following:
 * class Iterator {
 *   using reference = ...; // (Optional) Return type from dereference (*) operator.
 *   using pointer = ...; // (Optional) Return type from the -> operator.
 *   Iterator(); // (Optional) Default constructor
 *   static Iterator Iterator::begin(args...);
 *   static Iterator Iterator::end(args...);
 * };
 */
template <typename Iterator>
class Once {
  public:
    using value_type = typename Iterator::value_type;

    Once()
        requires std::is_default_constructible_v<Iterator>
        : begin_(), end_() {}

    template <typename... Args>
    explicit Once(Args &&...args)
        : begin_(Iterator::begin(std::forward<Args>(args)...)), end_(Iterator::end(std::forward<Args>(args)...)) {}

    Once(Iterator begin, Iterator end) : begin_(begin), end_(end) {}

    pure bool has_next() const { return begin_ != end_; }
    pure bool has_value() const { return begin_ != end_; }

    Once &operator++() {
        ++begin_;
        return *this;
    }

    pure typename Iterator::reference operator*() const
        requires HasDereferenceOperator<Iterator>
    {
        return begin_->operator*();
    }
    pure typename Iterator::pointer operator->() const
        requires HasMemberAccessOperator<Iterator>
    {
        return begin_.operator->();
    }

    pure Iterator &begin() { return begin_; }
    pure Iterator &end() { return end_; }

    pure const Iterator &begin() const { return begin_; }
    pure const Iterator &end() const { return end_; }

    pure bool operator==(const Once &rhs) const { return begin_ == rhs.begin_; }
    pure bool operator!=(const Once &rhs) const { return begin_ != rhs.begin_; }

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
};

} // namespace nox
