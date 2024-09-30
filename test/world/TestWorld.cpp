#include <gtest/gtest.h>

#include "nvl/actor/Actor.h"
#include "nvl/entity/Block.h"
#include "nvl/material/TestMaterial.h"
#include "nvl/world/World.h"

namespace {

using nvl::Actor;
using nvl::Block;
using nvl::Box;
using nvl::Color;
using nvl::Material;
using nvl::Pos;
using nvl::TestMaterial;
using nvl::World;

TEST(TestWorld, fall_out_of_bounds) {
    World<2>::Params params;
    params.maximum_y = 1;
    World<2> world(params);
    constexpr Box<2> box({0, 0}, {4, 4});
    const auto color = world.random.uniform<Color>(0, 255);
    const auto material = Material::get<TestMaterial>(color);
    const Actor actor = world.spawn<Block<2>>(Pos<2>::zero, box, material);
    const auto *block = actor.dyn_cast<Block<2>>();
    ASSERT_TRUE(block);
    EXPECT_EQ(block->bbox(), box);
    EXPECT_EQ(block->loc(), Pos<2>::zero);
    EXPECT_EQ(block->velocity(), Pos<2>::zero);
    EXPECT_EQ(block->accel(), Pos<2>::zero);

    world.tick({});
    EXPECT_EQ(block->loc(), world.kGravity);
    EXPECT_EQ(block->velocity(), world.kGravity);
    EXPECT_EQ(block->accel(), world.kGravity);

    for (I64 i = 0; i < 10; ++i) {
        world.tick({});
    }
    EXPECT_EQ(world.num_active(), 0);
    EXPECT_EQ(world.num_alive(), 0);
}

} // namespace
