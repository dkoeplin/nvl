#pragma once

#include "nvl/data/IteratorPair.h"
#include "nvl/data/Once.h"
#include "nvl/macros/Pure.h"

namespace nvl {

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
class Range : public IteratorPair<Range, Iterator, Iterator, Iterator> {
public:
    using parent = IteratorPair<Range, Iterator, Iterator, Iterator>;
    using value_type = typename Iterator::value_type;

    using parent::parent;

    template <typename... Args>
    explicit Range(Args &&...args)
        : parent(Iterator::begin(std::forward<Args>(args)...), Iterator::end(std::forward<Args>(args)...)) {}

    /// Returns a copy of this Range as a single iteration Once range.
    pure Once<Iterator> once() const { return Once<Iterator>(this->begin(), this->end()); }
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

} // namespace nvl
