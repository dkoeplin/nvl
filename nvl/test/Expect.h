#pragma once

#include <gtest/gtest.h>

#include "nvl/geo/Tuple.h"

namespace nvl::test {
#define EXPECT_ABOUT(actual, expected, error)                                                                          \
    do {                                                                                                               \
        for (size_t i = 0; i < actual.rank(); ++i) {                                                                   \
            EXPECT_NEAR(actual[i], expected[i], error);                                                                \
        }                                                                                                              \
    } while (false)

} // namespace nvl::test