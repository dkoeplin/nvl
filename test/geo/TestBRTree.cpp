#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "nvl/geo/Box.h"
#include "nvl/geo/BRTree.h"
#include "nvl/geo/Pos.h"
#include "nvl/test/LabeledBox.h"

namespace {

using testing::IsEmpty;
using testing::UnorderedElementsAre;

using nvl::At;
using nvl::Box;
using nvl::BRTree;
using nvl::Edge;
using nvl::List;
using nvl::Pos;
using nvl::Ref;
using nvl::Set;
using nvl::test::LabeledBox;

Set<At<2, Edge<2>>> view(List<Edge<2>> &list, const Pos<2> &offset) {
    Set<At<2, Edge<2>>> set;
    for (Edge<2> &item : list) {
        set.emplace(item, offset);
    }
    return set;
}

TEST(TestBRTree, create) {
    BRTree<2, LabeledBox> tree;
    tree.insert({1, {{0, 0}, {32, 32}}});

    EXPECT_EQ(tree.size(), 1);
    EXPECT_EQ(tree.nodes(), 1);
}

TEST(TestBRTree, fetch) {
    BRTree<2, LabeledBox> tree;
    auto box = tree.emplace(1, Box<2>({0, 0}, {32, 32}));

    const List<At<2, LabeledBox>> list(tree[{0, 0}]);
    EXPECT_THAT(list, UnorderedElementsAre(At<2, LabeledBox>(box, {0, 0})));
}

TEST(TestBRTree, move) {
    BRTree<2, LabeledBox> tree;
    const auto box = tree.emplace(1, Box<2>({0, 0}, {32, 32}));
    tree.loc = {500, 500};

    const List<At<2, LabeledBox>> list0(tree[Pos<2>::zero]);
    EXPECT_THAT(list0, IsEmpty());

    const List<At<2, LabeledBox>> list1(tree[tree.loc]);
    EXPECT_THAT(list1, UnorderedElementsAre(At<2, LabeledBox>(box, {500, 500})));
}

TEST(TestBRTree, edges) {
    BRTree<2, LabeledBox> tree;
    const auto box = tree.emplace(1, Box<2>({0, 0}, {32, 32}));
    List<Edge<2>> edges = box->bbox().edges();

    EXPECT_EQ(tree.edge_rtree().size(), 4);

    const Set edges0(tree.edges());
    const Set expect0 = view(edges, tree.loc);
    EXPECT_EQ(edges0, expect0);

    tree.loc = {500, 500};
    const Set edges1(tree.edges());
    const Set expect1 = view(edges, tree.loc);
    EXPECT_EQ(edges1, expect1);
}

} // namespace