#pragma once

#include "nvl/macros/Aliases.h"
#include "nvl/macros/Inline.h"

namespace nvl {

/// Integer grids are assumed to have the closest grid minimum to zero at zero exactly.
/// For example, with a grid size of 10, the grids are [-20, -11], [-10, -1], [0, 9], [10, 19], [20, 29], and so on.

/// Returns the closest integer grid maximum to `a` for a grid with size of `g`.
inline INLINE I64 grid_max(const I64 a, const I64 g) { return ((a < 0 ? a : (a + g)) / g) * g - 1; }

/// Returns the closest integer grid minimum to `a` for a grid with size of `b`.
inline INLINE I64 grid_min(const I64 a, const I64 g) { return ((a < 0 ? (a - g + 1) : a) / g) * g; }

} // namespace nvl
