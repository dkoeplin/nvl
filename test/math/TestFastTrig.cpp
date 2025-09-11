#include <gtest/gtest.h>

#include "nvl/math/FastTrig.h"
#include "nvl/math/Trig.h"
#include "nvl/test/Expect.h"

namespace {

using nvl::Pos;
using nvl::Vec;
using nvl::fasttrig::Deg;
using nvl::fasttrig::Rot;

TEST(TestFastTrig, rotate2d) {
    using nvl::fasttrig::rotate;
    constexpr Pos<2> x{0, 1};
    const F64 m = std::sin(nvl::PI / 4);
    EXPECT_ABOUT(rotate(x, {45}), Vec<2>(m, m), 1e-6);
    EXPECT_ABOUT(rotate(x, {-45}), Vec<2>(m, -m), 1e-6);
    EXPECT_ABOUT(rotate(x, {90}), Vec<2>(0, 1), 1e-6);
    EXPECT_ABOUT(rotate(x, {135}), Vec<2>(-m, m), 1e-6);
    EXPECT_ABOUT(rotate(x, {-135}), Vec<2>(-m, -m), 1e-6);
    EXPECT_ABOUT(rotate(x, {180}), Vec<2>(-1, 0), 1e-6);
    EXPECT_ABOUT(rotate(x, {225}), Vec<2>(-m, -m), 1e-6);
    EXPECT_ABOUT(rotate(x, {270}), Vec<2>(0, -1), 1e-6);
}

} // namespace
