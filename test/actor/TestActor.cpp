#include <gtest/gtest.h>

#include "nvl/actor/Actor.h"
#include "nvl/actor/TickResult.h"

namespace {

using nvl::AbstractActor;
using nvl::Actor;
using nvl::Box;
using nvl::Draw;
using nvl::List;
using nvl::Message;
using nvl::TickResult;

struct SimpleActor final : AbstractActor {
    class_tag(SimpleActor, AbstractActor);
    explicit SimpleActor(const Box<2> &box) : box_(box) {}

    TickResult tick(const List<Message> &) override { return {}; }
    void draw(Draw &, I64) const override {}

    Box<2> box_;
};

TEST(TestActor, construct) {
    SimpleActor actor(Box<2>({0, 0}, {32, 32}));
    actor.tick({});
}

} // namespace
