#pragma once

#include <iostream>

#include "nvl/macros/Aliases.h"

namespace nvl {

inline std::ostream &indented(const U64 n) {
    for (U64 i = 0; i < n; i++) {
        std::cout << "  ";
    }
    return std::cout;
}

} // namespace nvl
