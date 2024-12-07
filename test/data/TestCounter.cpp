#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "nvl/data/Counter.h"

namespace {

using nvl::Counter;
using nvl::List;
using testing::ElementsAre;

TEST(TestCounter, range) {
    const List<List<U64>> range(Counter<U64>::get(3, 2));
    const List<List<U64>> expected{{0, 0, 0}, {0, 0, 1}, {0, 1, 0}, {0, 1, 1},
                                   {1, 0, 0}, {1, 0, 1}, {1, 1, 0}, {1, 1, 1}};
    EXPECT_EQ(range, expected);
}

} // namespace
