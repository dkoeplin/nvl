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
        return Box<N>(a, b);
    }
    template <typename I>
    pure Box<N> normal(Random &random, const I mean, const I stddev) const {
        const auto a = random.normal<Pos<N>, I>(mean, stddev);
        const auto b = random.normal<Pos<N>, I>(mean, stddev);
        return Box<N>(a, b);
    }
};

namespace {

using testing::UnorderedElementsAre;

using nvl::Actor;
using nvl::Block;
using nvl::Box;
using nvl::Bulwark;
using nvl::Color;
using nvl::Face;
using nvl::Line;
using nvl::Material;
using nvl::Pos;
using nvl::Rel;
using nvl::TestMaterial;
using nvl::Vec;
using nvl::World;
using nvl::test::NullWindow;
using nvl::test::TensorWindow;

TEST(TestWorld, fall_out_of_bounds) {
    World<2>::Params params;
    params.maximum_y = 100;
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
        EXPECT_EQ(block->bbox().end[1], 0);
        EXPECT_EQ(world.num_awake(), 0);
        EXPECT_EQ(world.num_alive(), 2);
    }
}

TEST(TestWorld, first_2d) {
    NullWindow window;
    World<2> world(&window);
    constexpr Line<2> line({-2, 5}, {102, 5});
    constexpr Box<2> box({0, 0}, {10, 10});
    const auto material = Material::get<Bulwark>();
    const auto *block = world.spawn<Block<2>>(Pos<2>::zero, box, material);
    const auto intersect = world.first(line);
    ASSERT_TRUE(intersect.has_value());
    EXPECT_EQ(intersect->actor.ptr(), block);
    EXPECT_EQ(intersect->pt, Vec<2>(0, 5));
    EXPECT_EQ(intersect->face, Face(nvl::Dir::Neg, 0));
}

TEST(TestWorld, first_3d) {
    NullWindow window;
    World<3> world(&window);
    constexpr Line<3> line{{528, 969, 410}, {528, 974, 510}};
    constexpr Box<3> box{{500, 950, 500}, {549, 999, 549}};
    const auto material = Material::get<Bulwark>();
    const auto *block = world.spawn<Block<3>>(Pos<3>::zero, box, material);
    const auto intersect = world.first(line);
    ASSERT_TRUE(intersect.has_value());
    EXPECT_EQ(intersect->actor.ptr(), block);
    EXPECT_EQ(intersect->pt, Vec<3>(528, 973.5, 500));
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
    using nvl::Hit;
    using nvl::List;
    using nvl::Message;
    using nvl::Part;

    auto material = Material::get<TestMaterial>(Color::kBlack);
    material->falls = false;
    NullWindow window;
    auto *world = window.open<World<2>>();
    auto &block = *world->spawn<Block<2>>(Pos<2>::zero, Box<2>{{817, 846}, {1135, 1106}}, material);
    block.tick({Message::get<Hit<2>>(nullptr, Box<2>{{1100, 1005}, {1141, 1046}}, 1)});
    EXPECT_EQ(world->num_alive(), 1);
    block.tick({Message::get<Hit<2>>(nullptr, Box<2>{{1063, 1005}, {1104, 1046}}, 1)});
    EXPECT_EQ(world->num_alive(), 1);
    block.tick({Message::get<Hit<2>>(nullptr, Box<2>{{1024, 1005}, {1065, 1046}}, 1)});
    EXPECT_EQ(world->num_alive(), 1);

    nvl::Set<Box<2>> boxes;
    for (const Rel<Part<2>> &part : block.parts()) {
        boxes.insert(part.bbox(block.loc()));
    }
    EXPECT_THAT(boxes, UnorderedElementsAre(Box<2>{{817, 846}, {1024, 1106}}, Box<2>{{1024, 1046}, {1063, 1106}},
                                            Box<2>{{1024, 846}, {1063, 1005}}, Box<2>{{1063, 1046}, {1100, 1106}},
                                            Box<2>{{1063, 846}, {1100, 1005}}, Box<2>{{1100, 1046}, {1135, 1106}},
                                            Box<2>{{1100, 846}, {1135, 1005}}));

    block.tick({Message::get<Hit<2>>(nullptr, Box<2>{{987, 1006}, {1028, 1047}}, 1)});
    boxes.clear();
    for (auto actor : world->entities()) {
        auto *blk = actor.dyn_cast<Block<2>>();
        const Pos<2> loc = blk->loc();
        for (const Rel<Part<2>> &part : blk->parts()) {
            boxes.insert(part.bbox(loc));
        }
    }
    EXPECT_THAT(boxes, UnorderedElementsAre(Box<2>{{817, 846}, {987, 1106}}, Box<2>{{987, 1047}, {1024, 1106}},
                                            Box<2>{{987, 846}, {1024, 1006}}, Box<2>{{1028, 1046}, {1063, 1106}},
                                            Box<2>{{1024, 1047}, {1028, 1106}}, Box<2>{{1024, 846}, {1063, 1005}},
                                            Box<2>{{1063, 1046}, {1100, 1106}}, Box<2>{{1063, 846}, {1100, 1005}},
                                            Box<2>{{1100, 1046}, {1135, 1106}}, Box<2>{{1100, 846}, {1135, 1005}}));
    EXPECT_EQ(world->num_alive(), 1);
}

TEST(TestWorld, break_block2) {
    using nvl::Hit;
    using nvl::List;
    using nvl::Message;
    using nvl::Part;
    auto material = Material::get<TestMaterial>(Color::kBlack);
    material->falls = false;
    NullWindow window;
    auto *world = window.open<World<2>>();
    const List<Part<2>> parts{
        Part<2>({{1024, 846}, {1063, 1005}}, material),  Part<2>({{1024, 1046}, {1063, 1106}}, material),
        Part<2>({{817, 846}, {1024, 1106}}, material),   Part<2>({{1063, 846}, {1100, 1005}}, material),
        Part<2>({{1063, 1046}, {1100, 1106}}, material), Part<2>({{1100, 846}, {1135, 1005}}, material),
        Part<2>({{1100, 1046}, {1135, 1106}}, material),
    };
    auto &block = *world->spawn<Block<2>>(Pos<2>::zero, parts.range());
    block.tick({Message::get<Hit<2>>(nullptr, Box<2>{{987, 1006}, {1028, 1047}}, 1)});
    EXPECT_EQ(world->num_alive(), 1);
}

struct FuzzFall : nvl::test::FuzzingTestFixture<Box<2>, Pos<2>, Pos<2>, I64, I64> {
    FuzzFall() = default;
};

TEST_F(FuzzFall, fall2d) {
    using nvl::Distribution;
    using nvl::Random;

    this->num_tests = 1E3;
    this->in[0] = Distribution::Uniform<I64>(0, 100);
    this->in[1] = Distribution::Uniform<I64>(1, 30);
    this->in[2] = Distribution::Uniform<I64>(900, 999);
    this->in[3] = Distribution::Uniform<I64>(1, 100);

    auto material = Material::get<TestMaterial>(Color::kBlack);
    auto bulwark = Material::get<Bulwark>();

    fuzz([material, bulwark](Box<2> &end, const Pos<2> &loc, const Pos<2> &shape, const I64 y, const I64 thickness) {
        NullWindow window;
        auto *world = window.open<World<2>>();
        world->spawn<Block<2>>(Pos<2>(0, y), Pos<2>(1000, thickness), bulwark);
        const auto *block = world->spawn<Block<2>>(loc, shape, material);
        for (int64_t i = 0; i < 1000 && world->num_awake() > 0; ++i) {
            world->tick();
        }
        ASSERT(world->num_awake() == 0, "Failed to converge:" << " {loc: " << loc << ", shape: " << shape
                                                              << ", y: " << y << ", thickness: " << thickness << "}");
        end = block->bbox();
    });

    verify([&](const Box<2> &end, const Pos<2> &loc, const Pos<2> &shape, const I64 y, const I64 thickness) {
        ASSERT(end.end[1] == y, "Incorrect ending location! " << std::endl
                                                              << "Box: " << end << " {start: " << loc
                                                              << ", shape: " << shape << ", y: " << y
                                                              << ", thickness: " << thickness << "}");
    });
}

} // namespace
