#pragma once

#include "nvl/data/Ref.h"
#include "nvl/geo/Box.h"
#include "nvl/geo/HasBBox.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/Pure.h"

namespace nvl {

/**
 * @class View
 * @brief Views a Value at an offset within an N-dimensional space.
 *
 * Holds a reference to the value, and allows inspecting the bounding box of the value at the offset with .box().
 *
 * @tparam N Number of dimensions in the N-dimensional space
 * @tparam Value Value type being stored.
 */
template <U64 N, typename Value>
    requires traits::HasBBox<Value>
class View {
public:
    explicit View(Ref<Value> value, const Pos<N> &offset) : value_(value.ptr()), offset_(offset) {}
    explicit View(Value *value, const Pos<N> &offset) : value_(value), offset_(offset) {}

    pure bool operator==(const View &rhs) const { return *value_ == *rhs.value_ && offset_ == rhs.offset_; }
    pure bool operator!=(const View &rhs) const { return !(*this == rhs); }

    const Value *operator->() const { return value_; }
    const Value &operator*() const { return *value_; }

    pure Box<N> bbox() const { return value_->bbox() + offset_; }

    pure const Pos<N> &offset() const { return offset_; }

private:
    Value *value_;
    Pos<N> offset_;
};

template <U64 N, typename Value>
std::ostream &operator<<(std::ostream &os, const View<N, Value> &view) {
    return os << *view << " @ " << view.offset();
}

} // namespace nvl

template <U64 N, typename Value>
struct std::hash<nvl::View<N, Value>> {
    pure U64 operator()(const nvl::View<N, Value> &a) const { return std::hash<Value>()(*a); }
};
