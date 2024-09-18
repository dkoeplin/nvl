#include <gtest/gtest.h>

#include "nvl/actor/Actor.h"

namespace {

using nvl::Actor;
using nvl::Box;
using nvl::Draw;

struct SimpleActor final : Actor<2>::Impl {
    explicit SimpleActor(const Box<2> &box) : box_(box) {}

    void tick() override {}
    void draw(Draw &) const override {}
    pure Box<2> bbox() const override { return box_; }

    Box<2> box_;
};

TEST(TestActor, construct) {
    const auto actor = Actor<2>::get<SimpleActor>(Box<2>({0, 0}, {32, 32}));
    EXPECT_TRUE(actor.alive());
    EXPECT_EQ(actor.bbox(), Box<2>({0, 0}, {32, 32}));
}

} // namespace
