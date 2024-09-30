#include <gtest/gtest.h>

#include "nvl/entity/Entity.h"

namespace {

using nvl::Entity;
using nvl::Pos;
using nvl::Status;

struct SimpleEntity : Entity<2> {
    using Entity::Entity;
    Status broken(const nvl::List<Component> &) override { return Status::kNone; }
};

TEST(TestEntity, construct) {
    SimpleEntity entity(Pos<2>::zero);
    entity.tick({});
}

} // namespace
