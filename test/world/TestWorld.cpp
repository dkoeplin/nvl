#include <gtest/gtest.h>

#include "nvl/entity/Block.h"
#include "nvl/material/TestMaterial.h"
#include "nvl/world/World.h"

namespace {

using nvl::Block;
using nvl::Box;
using nvl::Color;
using nvl::Material;
using nvl::TestMaterial;
using nvl::World;

TEST(TestWorld, instantiate) {
    World<2> world;
    constexpr Box<2> box({0, 0}, {4, 4});
    const auto color = world.random.uniform<Color>(0, 255);
    const auto material = Material::get<TestMaterial>(color);
    world.spawn<Block<2>>(box, material);
}

} // namespace
