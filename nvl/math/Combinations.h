#pragma once

#include "nvl/macros/Aliases.h"

namespace nvl {

constexpr U64 factorial(const U64 n) { return n <= 1 ? 1 : n * factorial(n - 1); }

constexpr U64 combinations(const U64 n, const U64 r) { return factorial(n) / (factorial(r) * factorial(n - r)); }

} // namespace nvl