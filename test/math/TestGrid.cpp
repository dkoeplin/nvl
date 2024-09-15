#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "nvl/math/Grid.h"

namespace {

using nvl::grid_max;
using nvl::grid_min;

/// Integer grids are assumed to have the closest grid minimum to zero at zero exactly.
/// A single range in the grid is inclusive for both the minimum and maximum.
/// For example, with a grid size of 10, the grids are [-20, -11], [-10, -1], [0, 9], [10, 19], [20, 29], and so on.

TEST(TestGrid, grid_max) {
    EXPECT_EQ(grid_max(0, 10), 9);
    EXPECT_EQ(grid_max(10, 10), 19);
    EXPECT_EQ(grid_max(-3, 10), -1);
    EXPECT_EQ(grid_max(-9, 10), -1);
    EXPECT_EQ(grid_max(-10, 10), -1);
    EXPECT_EQ(grid_max(-19, 10), -11);
    EXPECT_EQ(grid_max(-20, 10), -11);
}

TEST(TestGrid, grid_min) {
    EXPECT_EQ(grid_min(0, 10), 0);
    EXPECT_EQ(grid_min(3, 10), 0);
    EXPECT_EQ(grid_min(10, 10), 10);
    EXPECT_EQ(grid_min(-1, 10), -10);
    EXPECT_EQ(grid_min(-5, 10), -10);
    EXPECT_EQ(grid_min(-9, 10), -10);
    EXPECT_EQ(grid_min(-10, 10), -10);
}

} // namespace
