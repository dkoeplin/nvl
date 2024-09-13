#pragma once

#include "nvl/macros/Aliases.h"

namespace nvl {

inline I64 clamp_up(const I64 a, const I64 b) { return ((a < 0 ? a : (a + b)) / b) * b - 1; }

inline I64 clamp_down(const I64 a, const I64 b) { return ((a < 0 ? (a - b + 1) : a) / b) * b; }

} // namespace nvl
