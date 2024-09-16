#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "nvl/math/Grid.h"
#include "nvl/math/Random.h"
#include "util/Fuzzing.h"

namespace {

using nvl::grid_max;
using nvl::grid_min;

/// Integer grids are assumed to have the closest grid minimum to zero at zero exactly.
/// A single range in the grid is inclusive for both the minimum and maximum.
/// For example, with a grid size of 10, the grids are [-20, -11], [-10, -1], [0, 9], [10, 19], [20, 29], and so on.

TEST(TestGrid, grid_max) {
    EXPECT_EQ(grid_max(0, 10), 9);
    EXPECT_EQ(grid_max(10, 10), 19);
    EXPECT_EQ(grid_max(-1, 10), -1);
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
    EXPECT_EQ(grid_min(-2, 10), -10);
    EXPECT_EQ(grid_min(-5, 10), -10);
    EXPECT_EQ(grid_min(-9, 10), -10);
    EXPECT_EQ(grid_min(-10, 10), -10);
}

using Output = std::pair<I64, I64>;
using TestGridFuzzer = nvl::testing::FuzzingTestFixture<Output, I64, I64>;
TEST_F(TestGridFuzzer, fuzz_grid) {
    N = 1E4;
    in[0] = nvl::Distribution::Uniform<I64>(-10000, 10000);
    in[1] = nvl::Distribution::Uniform<I64>(1, 1000);

    fuzz([](Output &output, const I64 a, const I64 g) {
        auto &[min, max] = output;
        max = grid_max(a, g);
        min = grid_min(a, g);
    });

    verify([&](const Output &output, const I64 a, const I64 g) {
        auto [min, max] = output;
        EXPECT_TRUE(min <= a);
        EXPECT_TRUE(a <= max);
        EXPECT_TRUE(min >= a - g);
        EXPECT_TRUE(a + g >= max);
        EXPECT_TRUE(min % g == 0);
        EXPECT_TRUE((max + 1) % g == 0);
        EXPECT_TRUE(max - min + 1 == g);
    });
}

} // namespace
