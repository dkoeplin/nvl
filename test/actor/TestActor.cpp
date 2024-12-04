#include <gtest/gtest.h>

#include "nvl/actor/Actor.h"
#include "nvl/geo/Volume.h"
#include "nvl/message/Message.h"

namespace {

using nvl::AbstractActor;
using nvl::Actor;
using nvl::Box;
using nvl::Color;
using nvl::List;
using nvl::Message;
using nvl::Status;
using nvl::Window;

struct SimpleActor final : AbstractActor {
    class_tag(SimpleActor, AbstractActor);
    explicit SimpleActor(const Box<2> &box) : box_(box) {}

    Status tick(const List<Message> &) override { return Status::kNone; }
    void draw(Window *, const Color &) const override {}

    Box<2> box_;
};

TEST(TestActor, construct) {
    auto actor = SimpleActor(Box<2>({0, 0}, {32, 32}));
    auto status = actor.tick({});
    EXPECT_EQ(status, nvl::Status::kNone);
}

} // namespace
