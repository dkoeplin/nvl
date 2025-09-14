#include <gtest/gtest.h>

#include "nvl/math/Deg.h"
#include "nvl/math/Trig.h"
#include "nvl/test/Expect.h"

namespace {

using nvl::Pos;
using nvl::Rotation;
using nvl::Vec;

TEST(TestTrig, rotate2d) {
    using nvl::rotate;
    constexpr Pos<2> x{0, 1};
    const F64 m = std::sin(nvl::PI / 4);
    EXPECT_VEC_NEAR(rotate(x, {45}), Vec<2>(-m, m), 1e-6);
    EXPECT_VEC_NEAR(rotate(x, {-45}), Vec<2>(m, m), 1e-6);
    EXPECT_VEC_NEAR(rotate(x, {90}), Vec<2>(-1, 0), 1e-6);
    EXPECT_VEC_NEAR(rotate(x, {135}), Vec<2>(-m, -m), 1e-6);
    EXPECT_VEC_NEAR(rotate(x, {-135}), Vec<2>(m, -m), 1e-6);
    EXPECT_VEC_NEAR(rotate(x, {180}), Vec<2>(0, -1), 1e-6);
    EXPECT_VEC_NEAR(rotate(x, {225}), Vec<2>(m, -m), 1e-6);
    EXPECT_VEC_NEAR(rotate(x, {270}), Vec<2>(1, 0), 1e-6);
}

} // namespace