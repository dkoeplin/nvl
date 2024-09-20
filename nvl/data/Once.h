#pragma once

#include "nvl/data/HasDereference.h"
#include "nvl/data/IteratorPair.h"
#include "nvl/macros/Pure.h"

namespace nvl {

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
class Once : public IteratorPair<Once, Iterator, Iterator &, const Iterator &> {
public:
    using parent = IteratorPair<Once, Iterator, Iterator &, const Iterator &>;
    using value_type = typename Iterator::value_type;

    using parent::parent;

    template <typename... Args>
    explicit Once(Args &&...args)
        : parent(Iterator::begin(std::forward<Args>(args)...), Iterator::end(std::forward<Args>(args)...)) {}

    pure bool has_next() const { return this->begin_ != this->end_; }
    pure bool has_value() const { return this->begin_ != this->end_; }

    Once &operator++() {
        ++this->begin_;
        return *this;
    }

    pure typename Iterator::reference operator*() const
        requires traits::HasDereferenceOperator<Iterator>
    {
        return this->begin_->operator*();
    }
    pure typename Iterator::pointer operator->() const
        requires traits::HasMemberAccessOperator<Iterator>
    {
        return this->begin_.operator->();
    }
};

} // namespace nvl
