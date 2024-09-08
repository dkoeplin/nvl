#pragma once

namespace nox {

#define return_if(cond, ...)                                                                                           \
    if ((cond)) {                                                                                                      \
        return __VA_ARGS__;                                                                                            \
    } // namespace nox

} // namespace nox
