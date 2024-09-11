#pragma once

#include "nox/macros/Aliases.h"
#include "nox/macros/Pure.h"

namespace nox::traits {

template <typename Value>
struct GetID final {
    constexpr GetID() = default;
    pure U64 operator()(const Value &value) const { return value.id; }
};

} // namespace nox::traits
