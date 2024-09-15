#pragma once

namespace nvl {

/// Enforces that the result of the function should not be discarded.
/// Also used to denote that this function has no side effects.
#define pure [[nodiscard]]

} // namespace nvl
