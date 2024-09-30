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

TEST(TestWorld, instantiate) {
    World<2> world;
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

    world.tick();
    EXPECT_EQ(block->velocity(), World<2>::kGravity);
    EXPECT_EQ(block->accel(), World<2>::kGravity);
}

} // namespace
