#pragma once

#include <iostream>

namespace nvl {

#define ASSERT(condition, message)                                                                                     \
    if (!(condition)) {                                                                                                \
        std::cerr << message << std::endl;                                                                             \
        std::abort();                                                                                                  \
    }

} // namespace nvl
