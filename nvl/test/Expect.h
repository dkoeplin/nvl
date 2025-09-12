#pragma once

#include <gtest/gtest.h>

#include "nvl/geo/Tuple.h"

namespace nvl::test {

#define EXPECT_VEC_NEAR(actual, expected, error)                                                                       \
    for (U64 i = 0; i < actual.rank(); ++i) {                                                                          \
        EXPECT_NEAR(actual[i], expected[i], error);                                                                    \
    }

} // namespace nvl::test