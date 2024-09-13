#pragma once

namespace nvl {

#define return_if(cond, ...)                                                                                           \
    if ((cond)) {                                                                                                      \
        return __VA_ARGS__;                                                                                            \
    } // namespace nvl

} // namespace nvl
