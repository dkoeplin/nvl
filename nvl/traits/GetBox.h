#pragma once

#include "nvl/geo/Box.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/Pure.h"

namespace nvl::traits {

template <U64 N, typename Value>
struct GetBox final {
    constexpr GetBox() = default;
    pure Box<N> operator()(const Value &value) const { return value.box; }
};

} // namespace nvl::traits
