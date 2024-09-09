#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "nox/data/RTree.h"
#include "nox/geo/Box.h"
#include "nox/geo/Pos.h"

namespace {

using testing::ElementsAre;
using testing::UnorderedElementsAre;

using nox::Box;
using nox::List;
using nox::Map;
using nox::RTree;
using nox::RTreeOptions;
using nox::Set;

struct LabeledBox {
    pure bool operator==(const LabeledBox &rhs) const { return id == rhs.id && box == rhs.box; }
    pure bool operator!=(const LabeledBox &rhs) const { return !(*this == rhs); }
    U64 id;
    Box<2> box;
};

std::ostream &operator<<(std::ostream &os, const LabeledBox &v) { return os << "BOX(" << v.id << ": " << v.box << ")"; }

TEST(TestRTree, create) {
    RTree<2, LabeledBox> tree;
    tree.insert({0, {{0, 5}, {5, 10}}});
    tree.testing().dump();
    EXPECT_EQ(tree.size(), 1);
    EXPECT_EQ(tree.nodes(), 1);
}

TEST(TestRTree, subdivide) {
    RTree<2, LabeledBox> tree({.max_entries = 2});
    const LabeledBox b0 {0, {{0, 5}, {10, 20}}};
    const LabeledBox b1 {1, {{10, 100}, {20, 120}}};
    const LabeledBox b2 {2, {{100, 200}, {200, 200}}};
    tree.insert(b0);
    tree.insert(b1);
    tree.insert(b2);

    tree.testing().dump();
    EXPECT_EQ(tree.size(), 3);  // Number of values
    EXPECT_EQ(tree.nodes(), 4); // Number of nodes

    const Map<Box<2>, Set<U64>> expected {
        {Box<2>({0, 0}, {127, 127}), Set<U64>{0, 1}},
        {Box<2>({0, 128}, {127, 255}), Set<U64>{2}},
        {Box<2>({128, 128}, {255, 255}), Set<U64>{2}}
    };
    EXPECT_EQ(tree.testing().collect_ids(), expected);

    // Check that we find all values when iterating over the bounding box, but each value is returned exactly once.
    const auto range = tree[tree.bounds()];
    const List<LabeledBox> elements (range.begin(), range.end());
    EXPECT_THAT(elements, UnorderedElementsAre(b0, b1, b2));
}

} // namespace
