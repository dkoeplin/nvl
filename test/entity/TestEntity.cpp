#include <gtest/gtest.h>

#include "nvl/entity/Entity.h"

namespace {

using nvl::Entity;

struct SimpleEntity : Entity<2> {};

TEST(TestEntity, construct) {
    SimpleEntity entity;
    entity.tick({});
}

} // namespace
