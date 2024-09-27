#pragma once

#include "nvl/data/Iterator.h"
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
template <typename Value, View Type = View::kImmutable>
class Once final {
public:
    using value_type = Value;

    Once() = default;
    Once(Iterator<Value, Type> begin, Iterator<Value, Type> end) : begin_(begin), end_(end) {}

    pure Iterator<Value, Type> &begin() { return begin_; }
    pure Iterator<Value, Type> &end() { return end_; }
    pure const Iterator<Value, Type> &begin() const { return begin_; }
    pure const Iterator<Value, Type> &end() const { return end_; }

    pure bool empty() const { return begin_ == end_; }
    pure bool has_next() const { return begin_ != this->end_; }
    pure bool has_value() const { return begin_ != this->end_; }

    pure bool operator==(const Once &rhs) const { return begin_ == rhs.begin_; }
    pure bool operator!=(const Once &rhs) const { return begin_ != rhs.begin_; }

    Once &operator++() {
        ++this->begin_;
        return *this;
    }

    pure const Value &operator*() const { return *begin_; }
    pure const Value *operator->() const { return &*begin_; }

private:
    Iterator<Value, Type> begin_;
    Iterator<Value, Type> end_;
};

template <typename IterType, View Type = View::kImmutable, typename... Args>
Once<typename IterType::value_type, Type> make_once(Args &&...args) {
    auto i0 = IterType::template begin<Type>(std::forward<Args>(args)...);
    auto i1 = IterType::template end<Type>(std::forward<Args>(args)...);
    return {i0, i1};
}

} // namespace nvl
