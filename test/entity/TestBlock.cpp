#include <gtest/gtest.h>

#include "nvl/entity/Block.h"
#include "nvl/entity/Entity.h"
#include "nvl/geo/Tuple.h"
#include "nvl/geo/Volume.h"
#include "nvl/material/TestMaterial.h"
#include "nvl/message/Hit.h"
#include "nvl/test/Fuzzing.h"

namespace {

using nvl::Block;
using nvl::Box;
using nvl::Color;
using nvl::Distribution;
using nvl::Entity;
using nvl::Hit;
using nvl::List;
using nvl::Material;
using nvl::Message;
using nvl::Pos;
using nvl::Rel;
using nvl::Set;
using nvl::Status;
using nvl::TestMaterial;
using nvl::Window;
using nvl::World;

Set<Box<2>> boxes(const World<2> &world) {
    Set<Box<2>> boxes;
    for (const auto &actor : world.entities()) {
        if (auto *entity = actor.dyn_cast<Entity<2>>()) {
            for (const auto &part : entity->parts()) {
                boxes.insert(part.bbox(entity->loc()));
            }
        }
    }
    return boxes;
}

Set<Box<2>> ref(const Pos<2> &box_loc, const Pos<2> &box_shape, const Pos<2> &hit_loc, const Pos<2> &hit_shape) {
    const Box<2> box(box_loc, box_loc + box_shape);
    const Box<2> hitbox(hit_loc, hit_loc + hit_shape);
    return Set<Box<2>>(box.diff(hitbox).range());
}

Set<Box<2>> hit(const Pos<2> &box_loc, const Pos<2> &box_shape, const Pos<2> &hit_loc, const Pos<2> &hit_shape) {
    World<2>::Params params;
    params.maximum_y = 5000;
    params.gravity_accel = 0;
    World<2> world(nullptr, params);

    const auto material = Material::get<TestMaterial>(Color::kBlue);
    auto block = world.spawn<Block<2>>(box_loc, box_shape, material);
    auto hit = Message::get<Hit<2>>(nullptr, Box<2>(hit_loc, hit_loc + hit_shape), material->durability);
    block->tick({hit});
    return boxes(world);
}

// [3] Hit {{1026, 1073}, {1066, 1113}}
//     {{737, 863}, {1146, 1150}}11
// [4] Hit {{1087, 901}, {1127, 941}}
//     {{737, 863}, {1146, 1150}}

TEST(TestBlock, hit_block0) {
    constexpr Pos<2> box_loc{737, 863};
    constexpr Pos<2> box_shape{409, 287};
    constexpr Pos<2> hit_loc{1026, 1073};
    constexpr Pos<2> hit_shape{40, 40};
    const auto actual = hit(box_loc, box_shape, hit_loc, hit_shape);
    const auto expected = ref(box_loc, box_shape, hit_loc, hit_shape);
    EXPECT_EQ(actual, expected);
}

struct FuzzHitBlock : nvl::test::FuzzingTestFixture<Set<Box<2>>, Pos<2>, Pos<2>, Pos<2>, Pos<2>> {};

TEST_F(FuzzHitBlock, hit_block2d) {
    this->num_tests = 1E6;
    this->in[0] = Distribution::Uniform<I64>(0, 1000); // Box offset
    this->in[1] = Distribution::Uniform<I64>(1, 1000); // Box size
    this->in[2] = Distribution::Uniform<I64>(0, 1000); // Hit offset
    this->in[3] = Distribution::Uniform<I64>(0, 500);  // Hit size

    const auto material = Material::get<TestMaterial>(Color::kBlue);

    fuzz([&](Set<Box<2>> &result, const Pos<2> &box_loc, const Pos<2> &box_shape, const Pos<2> &hit_loc,
             const Pos<2> &hit_shape) { result = hit(box_loc, box_shape, hit_loc, hit_shape); });

    verify([&](const Set<Box<2>> &result, const Pos<2> &box_loc, const Pos<2> &box_shape, const Pos<2> &hit_loc,
               const Pos<2> &hit_shape) {
        const Set<Box<2>> expected = ref(box_loc, box_shape, hit_loc, hit_shape);

        EXPECT_EQ(expected, result);
    });
}

} // namespace
