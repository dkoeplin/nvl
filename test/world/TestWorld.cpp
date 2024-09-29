#include <gtest/gtest.h>

#include "nvl/entity/Block.h"
#include "nvl/world/World.h"

namespace {

using nvl::Block;
using nvl::Box;
using nvl::Color;
using nvl::World;

TEST(TestWorld, instantiate) {
    constexpr Box<2> box({0, 0}, {4, 4});

    World<2> world;
    world.spawn<Block<2>>(box, world.random.uniform<Color>(0, 255));
}

} // namespace
