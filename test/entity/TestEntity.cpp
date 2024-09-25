#include <gtest/gtest.h>

#include "nvl/entity/Entity.h"

namespace {

using nvl::Entity;
using nvl::Status;

struct SimpleEntity : Entity<2> {
    Status broken(const nvl::List<Component> &) override { return Status::kNone; }
};

TEST(TestEntity, construct) {
    SimpleEntity entity;
    entity.tick({});
}

} // namespace
