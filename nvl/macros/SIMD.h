#pragma once

namespace nvl {

/// Requires clang to vectorize the following for or while loop.
/// An unvectorizable loop will result in a clang compiler warning.
#define simd _Pragma("clang loop vectorize(enable)")

/// Forbid SIMD transformations on the following for or while loop.
#define no_simd _Pragma("clang loop vectorize(disable)")

} // namespace nvl
