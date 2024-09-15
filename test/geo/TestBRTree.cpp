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
using nvl::Set;
using nvl::View;
using nvl::testing::LabeledBox;

TEST(TestBRTree, create) {
    BRTree<2, LabeledBox> tree;
    tree.insert({1, {{0, 0}, {32, 32}}});

    EXPECT_EQ(tree.size(), 1);
    EXPECT_EQ(tree.debug.nodes(), 1);
}

TEST(TestBRTree, fetch) {
    BRTree<2, LabeledBox> tree;
    LabeledBox box{1, {{0, 0}, {32, 32}}};
    tree.insert(box);

    const List<View<2, LabeledBox>> list(tree[{0, 0}]);
    EXPECT_THAT(list, UnorderedElementsAre(View<2, LabeledBox>(box, {0, 0})));
}

TEST(TestBRTree, move) {
    BRTree<2, LabeledBox> tree;
    LabeledBox box{1, {{0, 0}, {32, 32}}};
    tree.insert(box);
    tree.loc = {500, 500};

    const List<View<2, LabeledBox>> list0(tree[{0, 0}]);
    EXPECT_THAT(list0, IsEmpty());

    const List<View<2, LabeledBox>> list1(tree[{500, 500}]);
    EXPECT_THAT(list1, UnorderedElementsAre(View<2, LabeledBox>(box, {500, 500})));
}

Set<View<2, Edge<2>>> view(List<Edge<2>> &list, const Pos<2> &offset) {
    Set<View<2, Edge<2>>> set;
    for (Edge<2> &item : list) {
        set.emplace(item, offset);
    }
    return set;
}

TEST(TestBRTree, borders) {
    BRTree<2, LabeledBox> tree;
    LabeledBox box{1, {{0, 0}, {32, 32}}};
    List<Edge<2>> borders = box.box().borders();

    tree.insert(box);
    const Set<View<2, Edge<2>>> borders0(tree.borders());
    const Set<View<2, Edge<2>>> expected0 = view(borders, tree.loc);
    EXPECT_EQ(borders0, expected0);

    tree.loc = {500, 500};
    const Set<View<2, Edge<2>>> borders1(tree.borders());
    const Set<View<2, Edge<2>>> expected1 = view(borders, tree.loc);
    EXPECT_EQ(borders1, expected1);
}

} // namespace