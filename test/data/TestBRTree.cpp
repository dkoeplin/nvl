#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "nvl/data/BRTree.h"
#include "nvl/geo/Box.h"
#include "nvl/geo/Pos.h"
#include "util/LabeledBox.h"

namespace {

using testing::ElementsAre;
using testing::IsEmpty;
using testing::UnorderedElementsAre;

using nvl::Box;
using nvl::BRTree;
using nvl::List;
using nvl::Pos;
using nvl::testing::LabeledBox;

TEST(TestBRTree, create) {

}

} // namespace