#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "nvl/data/UnionFind.h"

namespace {

using testing::UnorderedElementsAre;

using nvl::Set;
using nvl::UnionFind;

TEST(TestUnionFind, basic) {
    UnionFind<U64> sets;
    sets.add(0, 2);
    sets.add(4, 5);
    sets.add(0, 1);
    sets.add(6, 7);
    EXPECT_THAT(sets.sets(), UnorderedElementsAre(Set<U64>{0, 1, 2}, Set<U64>{4, 5}, Set<U64>{6, 7}));
}

} // namespace