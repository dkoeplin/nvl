#pragma once

#include <optional>

#include "nvl/macros/Implicit.h"
#include "nvl/macros/Pure.h"

namespace nvl {

template <typename Value> using Maybe = std::optional<Value>;

constexpr std::nullopt_t None = std::nullopt;

template <typename Value> Maybe<Value> Some(const Value &v) { return Maybe<Value>(v); }
template <typename Value> Maybe<Value> Some(Value &&v) { return Maybe<Value>(v); }

} // namespace nvl