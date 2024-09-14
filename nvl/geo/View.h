#pragma once

#include "nvl/data/Ref.h"
#include "nvl/geo/Box.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/Pure.h"
#include "nvl/traits/HasBox.h"
#include "nvl/traits/HasID.h"

namespace nvl {

template <U64 N, typename Value>
    requires traits::HasBox<Value> && traits::HasID<Value>
class View {
public:
    explicit View(Value &value, const Pos<N> &offset) : value_(value), offset_(offset) {}

    pure bool operator==(const View &rhs) const { return value_ == rhs.value_ && offset_ == rhs.offset_; }
    pure bool operator!=(const View &rhs) const { return !(*this == rhs); }

    const Value *operator->() const { return &value_; }
    const Value &operator*() const { return value_; }

    pure Box<N> box() const { return value_.box() + offset_; }
    pure U64 id() const { return value_.id(); }

private:
    Ref<Value> value_;
    Pos<N> offset_;
};

} // namespace nvl
