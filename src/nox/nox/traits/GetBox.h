#pragma once

#include "nox/geo/Box.h"
#include "nox/macros/Aliases.h"
#include "nox/macros/Pure.h"

namespace nox::traits {

template <U64 N, typename Value>
struct GetBox final {
    constexpr GetBox() = default;
    pure Box<N> operator()(const Value &value) const { return value.box; }
};

} // namespace nox::traits
