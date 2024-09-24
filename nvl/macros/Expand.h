#pragma once

#include <sys/cdefs.h>

namespace nvl {

/// Stronger hint than the standard inline keyword which tells the compiler to "always" inline this function.
/// The C++ "inline" keyword relates to the location of function definitions. not inlining as an optimization.
#define expand __header_always_inline

} // namespace nvl
