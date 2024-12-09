#pragma once

#include <optional>

#include "nvl/macros/ReturnIf.h"

namespace nvl {

template <typename Value>
using Maybe = std::optional<Value>;

constexpr std::nullopt_t None = std::nullopt;

template <typename Value>
Maybe<Value> Some(const Value &v) {
    return Maybe<Value>(v);
}
template <typename Value>
Maybe<Value> Some(Value &&v) {
    return Maybe<Value>(v);
}

template <typename T>
Maybe<T> SomeIf(const T x, const bool enable) {
    return_if(enable, x);
    return None;
}

template <typename T>
    requires std::is_integral_v<T> || std::is_floating_point_v<T>
Maybe<T> max(Maybe<T> a, Maybe<T> b) {
    return_if(a.has_value() && b.has_value(), std::max(*a, *b));
    return_if(a.has_value(), a);
    return b;
}

template <typename T>
    requires std::is_integral_v<T> || std::is_floating_point_v<T>
Maybe<T> min(Maybe<T> a, Maybe<T> b) {
    return_if(a.has_value() && b.has_value(), std::min(*a, *b));
    return_if(a.has_value(), a);
    return b;
}

} // namespace nvl