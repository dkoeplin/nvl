#include <gtest/gtest.h>

#include "nvl/math/Distribution.h"
#include "nvl/math/Random.h"

namespace {

using nvl::Distribution;
using nvl::Random;

TEST(TestDistribution, next) {
    Random random;
    auto dist = Distribution::Uniform<U64>(0, 32);
    auto next = dist.next<U64>(random);
    EXPECT_TRUE(next <= 32);
}

} // namespace
