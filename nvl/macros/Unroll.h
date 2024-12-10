#pragma once

namespace nvl {

#define unroll _Pragma("clang loop unroll(enable)")
#define no_unroll _Pragma("clang loop unroll(disable)")

} // namespace nvl