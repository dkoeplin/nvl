#include <gtest/gtest.h>

#include "nvl/entity/Entity.h"

namespace {

using nvl::Entity;
using nvl::Pos;
using nvl::Status;
using nvl::Window;

struct SimpleEntity : Entity<2> {
    using Entity::Entity;
    Status broken(const nvl::List<Component> &) override { return Status::kNone; }
    void draw(Window &, U64) const override {}
};

TEST(TestEntity, construct) {
    SimpleEntity entity(Pos<2>::zero);
    entity.tick({});
}

} // namespace
