#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "nox/data/RTree.h"
#include "nox/geo/Box.h"
#include "nox/geo/Pos.h"

namespace {

using testing::ElementsAre;

using nox::Box;
using nox::RTree;
using nox::RTreeOptions;

struct LabeledBox {
    U64 id;
    Box<2> box;
};

std::ostream &operator<<(std::ostream &os, const LabeledBox &v) { return os << "BOX(" << v.id << ": " << v.box << ")"; }

TEST(TestRTree, create) {
    RTree<2, LabeledBox> tree;
    tree.insert({0, {{0, 5}, {5, 10}}});
    tree.dump();
    EXPECT_EQ(tree.size(), 1);
    EXPECT_EQ(tree.nodes(), 1);
}

TEST(TestRTree, subdivide) {
    RTree<2, LabeledBox> tree({.max_entries = 2});
    tree.insert({0, {{0, 5}, {10, 20}}});
    tree.insert({1, {{10, 100}, {20, 120}}});
    tree.insert({2, {{100, 200}, {200, 200}}});

    tree.dump();
    EXPECT_EQ(tree.size(), 3);  // Number of values
    EXPECT_EQ(tree.nodes(), 4); // Number of nodes
}

} // namespace
