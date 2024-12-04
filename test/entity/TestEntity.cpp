#include <gtest/gtest.h>

#include "nvl/entity/Block.h"
#include "nvl/entity/Entity.h"
#include "nvl/geo/Tuple.h"
#include "nvl/geo/Volume.h"
#include "nvl/material/TestMaterial.h"
#include "nvl/message/Hit.h"

namespace {

using nvl::Block;
using nvl::Box;
using nvl::Color;
using nvl::Entity;
using nvl::Hit;
using nvl::Material;
using nvl::Message;
using nvl::Pos;
using nvl::Status;
using nvl::TestMaterial;
using nvl::Window;

struct SimpleEntity : Entity<2> {
    using Entity::Entity;
    Status broken(const nvl::List<Component> &) override { return Status::kNone; }
    void draw(Window *, const Color &) const override {}
};

TEST(TestEntity, construct) {
    SimpleEntity entity(Pos<2>::zero);
    entity.tick({});
}

} // namespace
