#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "nvl/geo/Box.h"
#include "nvl/geo/BRTree.h"
#include "nvl/geo/Pos.h"
#include "util/LabeledBox.h"

namespace {

using testing::IsEmpty;
using testing::UnorderedElementsAre;

using nvl::Box;
using nvl::BRTree;
using nvl::Edge;
using nvl::List;
using nvl::Pos;
using nvl::Ref;
using nvl::Set;
using nvl::View;
using nvl::testing::LabeledBox;

Set<View<2, Edge<2>>> view(List<Edge<2>> &list, const Pos<2> &offset) {
    Set<View<2, Edge<2>>> set;
    for (Edge<2> &item : list) {
        set.emplace(item, offset);
    }
    return set;
}

TEST(TestBRTree, create) {
    BRTree<2, LabeledBox> tree;
    tree.insert({1, {{0, 0}, {32, 32}}});

    EXPECT_EQ(tree.size(), 1);
    EXPECT_EQ(tree.debug.nodes(), 1);
}

TEST(TestBRTree, fetch) {
    BRTree<2, LabeledBox> tree;
    auto box = tree.emplace(1, Box<2>({0, 0}, {32, 32}));

    const List<View<2, LabeledBox>> list(tree[{0, 0}]);
    EXPECT_THAT(list, UnorderedElementsAre(View<2, LabeledBox>(box, {0, 0})));
}

TEST(TestBRTree, move) {
    BRTree<2, LabeledBox> tree;
    auto box = tree.emplace(1, Box<2>({0, 0}, {32, 32}));
    tree.loc = {500, 500};

    const List<View<2, LabeledBox>> list0(tree[{0, 0}]);
    EXPECT_THAT(list0, IsEmpty());

    const List<View<2, LabeledBox>> list1(tree[{500, 500}]);
    EXPECT_THAT(list1, UnorderedElementsAre(View<2, LabeledBox>(box, {500, 500})));
}

TEST(TestBRTree, edges) {
    BRTree<2, LabeledBox> tree;
    auto box = tree.emplace(1, Box<2>({0, 0}, {32, 32}));
    List<Edge<2>> edges = box->bbox().edges();

    EXPECT_EQ(tree.debug.edge_rtree().size(), 4);

    const Set<View<2, Edge<2>>> edges0(tree.unordered_edges());
    const Set<View<2, Edge<2>>> expected0 = view(edges, tree.loc);
    EXPECT_EQ(edges0, expected0);

    tree.loc = {500, 500};
    const Set<View<2, Edge<2>>> edges1(tree.unordered_edges());
    const Set<View<2, Edge<2>>> expected1 = view(edges, tree.loc);
    EXPECT_EQ(edges1, expected1);
}

} // namespace