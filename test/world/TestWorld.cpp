#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <utility>

#include "nvl/actor/Actor.h"
#include "nvl/entity/Block.h"
#include "nvl/material/Bulwark.h"
#include "nvl/material/TestMaterial.h"
#include "nvl/test/Fuzzing.h"
#include "nvl/test/NullWindow.h"
#include "nvl/test/TensorWindow.h"
#include "nvl/world/World.h"

template <U64 N>
struct nvl::RandomGen<nvl::Box<N>> {
    template <typename I>
    pure Box<N> uniform(Random &random, const I min, const I max) const {
        const auto a = random.uniform<Pos<N>, I>(min, max);
        const auto b = random.uniform<Pos<N>, I>(min, max);
        return Box(a, b);
    }
    template <typename I>
    pure Box<N> normal(Random &random, const I mean, const I stddev) const {
        const auto a = random.normal<Pos<N>, I>(mean, stddev);
        const auto b = random.normal<Pos<N>, I>(mean, stddev);
        return Box(a, b);
    }
};

namespace {

using testing::UnorderedElementsAre;

using nvl::Actor;
using nvl::Block;
using nvl::Box;
using nvl::Bulwark;
using nvl::Color;
using nvl::Material;
using nvl::Pos;
using nvl::TestMaterial;
using nvl::World;
using nvl::test::NullWindow;
using nvl::test::TensorWindow;

TEST(TestWorld, fall_out_of_bounds) {
    World<2>::Params params;
    params.maximum_y = 1;
    params.gravity_accel = 10;
    World<2> world(nullptr, params);
    constexpr Box<2> box({0, 0}, {4, 4});
    const auto color = world.random.uniform<Color>(0, 255);
    const auto material = Material::get<TestMaterial>(color);
    const auto *block = world.spawn<Block<2>>(Pos<2>::zero, box, material);
    ASSERT_TRUE(block);
    EXPECT_EQ(block->bbox(), box);
    EXPECT_EQ(block->loc(), Pos<2>::zero);
    EXPECT_EQ(block->velocity(), Pos<2>::zero);
    EXPECT_EQ(block->accel(), Pos<2>::zero);

    world.tick();
    EXPECT_EQ(block->loc(), world.kGravity);
    EXPECT_EQ(block->velocity(), world.kGravity);
    EXPECT_EQ(block->accel(), world.kGravity);

    // Run for enough ticks for the block to fall out of bounds
    for (I64 i = 0; i < 10; ++i) {
        world.tick();
    }
    // Check that it is now dead and removed
    EXPECT_EQ(world.num_awake(), 0);
    EXPECT_EQ(world.num_alive(), 0);
}

TEST(TestWorld, idle_when_not_moving) {
    NullWindow window;
    World<2> world(&window);
    {
        constexpr Box<2> box({0, 0}, {10, 2});
        const auto material = Material::get<Bulwark>();
        const auto *block = world.spawn<Block<2>>(Pos<2>::zero, box, material);
        world.tick();
        EXPECT_EQ(block->loc(), Pos<2>::zero);
    }
    EXPECT_EQ(world.num_awake(), 0);
    EXPECT_EQ(world.num_alive(), 1);
    {
        constexpr Box<2> box({0, 0}, {5, 5});
        const auto color = world.random.uniform<Color>(0, 255);
        const auto material = Material::get<TestMaterial>(color);
        const auto *block = world.spawn<Block<2>>(Pos<2>{5, -50}, box, material);

        for (U64 i = 0; i < 10; ++i) {
            world.tick();
        }
        EXPECT_EQ(block->bbox().max[1], -1);
        EXPECT_EQ(world.num_awake(), 0);
        EXPECT_EQ(world.num_alive(), 2);
    }
}

TEST(TestWorld, stop_when_fallen) {
    TensorWindow window("stop_when_fallen", {10, 10});
    World<2>::Params params;
    params.gravity_accel = 3;
    params.maximum_y = 15;
    auto *world = window.open<World<2>>();
    world->set_hud(false);
    auto test_material = Material::get<TestMaterial>(Color::kBlack);
    auto bulwark = Material::get<Bulwark>();
    test_material->outline = false;
    bulwark->outline = false;
    world->spawn<Block<2>>(Pos<2>(4, 0), Box<2>({0, 0}, {0, 0}), test_material);
    world->spawn<Block<2>>(Pos<2>(0, 8), Box<2>({0, 0}, {9, 0}), bulwark);
    for (I64 i = 0; i < 10; ++i) {
        window.draw();
        std::cout << "Tick #" << i << ": " << std::endl;
        nvl::test::print_10x10_tensor(window.tensor());
        window.tick();
    }
}

TEST(TestWorld, break_block) {
    using nvl::At;
    using nvl::Hit;
    using nvl::List;
    using nvl::Message;
    using nvl::Part;

    auto material = Material::get<TestMaterial>(Color::kBlack);
    material->falls = false;
    NullWindow window;
    auto *world = window.open<World<2>>();
    auto &block = *world->spawn<Block<2>>(Pos<2>::zero, Box<2>{{817, 846}, {1134, 1105}}, material);
    block.tick({Message::get<Hit<2>>(nullptr, Box<2>{{1100, 1005}, {1140, 1045}}, 1)});
    EXPECT_EQ(world->num_alive(), 1);
    block.tick({Message::get<Hit<2>>(nullptr, Box<2>{{1063, 1005}, {1103, 1045}}, 1)});
    EXPECT_EQ(world->num_alive(), 1);
    block.tick({Message::get<Hit<2>>(nullptr, Box<2>{{1024, 1005}, {1064, 1045}}, 1)});
    EXPECT_EQ(world->num_alive(), 1);

    nvl::Set<Box<2>> boxes;
    for (const At<2, Part<2>> &part : block.parts()) {
        boxes.insert(part.bbox());
    }
    EXPECT_THAT(boxes, UnorderedElementsAre(Box<2>{{817, 846}, {1023, 1105}}, Box<2>{{1024, 1046}, {1062, 1105}},
                                            Box<2>{{1024, 846}, {1062, 1004}}, Box<2>{{1063, 1046}, {1099, 1105}},
                                            Box<2>{{1063, 846}, {1099, 1004}}, Box<2>{{1100, 1046}, {1134, 1105}},
                                            Box<2>{{1100, 846}, {1134, 1004}}));

    block.tick({Message::get<Hit<2>>(nullptr, Box<2>{{987, 1006}, {1027, 1046}}, 1)});
    boxes.clear();
    for (auto actor : world->entities()) {
        for (const At<2, Part<2>> &part : actor.dyn_cast<Block<2>>()->parts()) {
            boxes.insert(part.bbox());
        }
    }
    EXPECT_THAT(boxes, UnorderedElementsAre(Box<2>{{817, 846}, {986, 1105}}, Box<2>{{987, 1047}, {1023, 1105}},
                                            Box<2>{{987, 846}, {1023, 1005}}, Box<2>{{1028, 1046}, {1062, 1105}},
                                            Box<2>{{1024, 1047}, {1027, 1105}}, Box<2>{{1024, 846}, {1062, 1004}},
                                            Box<2>{{1063, 1046}, {1099, 1105}}, Box<2>{{1063, 846}, {1099, 1004}},
                                            Box<2>{{1100, 1046}, {1134, 1105}}, Box<2>{{1100, 846}, {1134, 1004}}));
    EXPECT_EQ(world->num_alive(), 1);
}

TEST(TestWorld, break_block2) {
    using nvl::At;
    using nvl::Hit;
    using nvl::List;
    using nvl::Message;
    using nvl::Part;
    auto material = Material::get<TestMaterial>(Color::kBlack);
    material->falls = false;
    NullWindow window;
    auto *world = window.open<World<2>>();
    const List<Part<2>> parts{
        Part<2>({{1024, 846}, {1062, 1004}}, material),  Part<2>({{1024, 1046}, {1062, 1105}}, material),
        Part<2>({{817, 846}, {1023, 1105}}, material),   Part<2>({{1063, 846}, {1099, 1004}}, material),
        Part<2>({{1063, 1046}, {1099, 1105}}, material), Part<2>({{1100, 846}, {1134, 1004}}, material),
        Part<2>({{1100, 1046}, {1134, 1105}}, material),
    };
    auto &block = *world->spawn<Block<2>>(Pos<2>::zero, parts.range());
    block.tick({Message::get<Hit<2>>(nullptr, Box<2>{{987, 1006}, {1027, 1046}}, 1)});
    EXPECT_EQ(world->num_alive(), 1);
}

struct FuzzFall : nvl::test::FuzzingTestFixture<Box<2>, Pos<2>, Box<2>, I64, I64> {
    FuzzFall() = default;
};

TEST_F(FuzzFall, fall2d) {
    using nvl::Distribution;
    using nvl::Random;

    this->num_tests = 1E3;
    this->in[0] = Distribution::Uniform<I64>(0, 100);
    this->in[1] = Distribution::Uniform<I64>(0, 30);
    this->in[2] = Distribution::Uniform<I64>(900, 999);
    this->in[3] = Distribution::Uniform<I64>(1, 100);

    auto material = Material::get<TestMaterial>(Color::kBlack);
    auto bulwark = Material::get<Bulwark>();

    fuzz([material, bulwark](Box<2> &end, const Pos<2> &loc, const Box<2> &shape, I64 y, I64 thickness) {
        NullWindow window;
        auto *world = window.open<World<2>>();
        world->spawn<Block<2>>(Pos<2>(0, y), Box<2>({0, 0}, {999, thickness}), bulwark);
        const auto *block = world->spawn<Block<2>>(loc, shape, material);
        for (int64_t i = 0; i < 1000 && world->num_awake() > 0; ++i) {
            world->tick();
        }
        ASSERT(world->num_awake() == 0, "Failed to converge:"
                                            << " {loc: " << loc << ", shape: " << shape << ", y: " << y
                                            << ", thickness: " << thickness << "}");
        end = block->bbox();
    });

    verify([&](const Box<2> &end, const Pos<2> &loc, const Box<2> &shape, I64 y, I64 thickness) {
        EXPECT_EQ(end.max[1], y - 1) << "Incorrect ending location! "
                                     << "Box: " << end << " {loc: " << loc << ", shape: " << shape << ", y: " << y
                                     << ", thickness: " << thickness << "}";
    });
}

} // namespace
