#pragma once

#include <optional>

#include "nox/macros/Implicit.h"
#include "nox/macros/Pure.h"

namespace nox {

template <typename Value> using Maybe = std::optional<Value>;

constexpr std::nullopt_t None = std::nullopt;

template <typename Value> Maybe<Value> Some(const Value &v) { return Maybe<Value>(v); }
template <typename Value> Maybe<Value> Some(Value &&v) { return Maybe<Value>(v); }

} // namespace nox