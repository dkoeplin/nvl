#pragma once

#include "nvl/macros/Aliases.h"
#include "nvl/macros/Pure.h"

namespace nvl::traits {

template <typename Value>
struct GetID final {
    constexpr GetID() = default;
    pure U64 operator()(const Value &value) const { return value.id; }
};

} // namespace nvl::traits
