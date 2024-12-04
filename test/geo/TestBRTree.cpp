#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "nvl/geo/BRTree.h"
#include "nvl/geo/Tuple.h"
#include "nvl/geo/Volume.h"
#include "nvl/math/Random.h"
#include "nvl/test/Fuzzing.h"
#include "nvl/test/LabeledBox.h"

template <U64 N>
struct nvl::RandomGen<nvl::Box<N>> {
    template <typename I>
    pure Box<N> uniform(Random &random, const I min, const I max) const {
        const auto a = random.uniform<Pos<N>, I>(min, max);
        const auto b = random.uniform<Pos<N>, I>(min, max);
        return Box<2>(a, b);
    }
    template <typename I>
    pure Box<N> normal(Random &random, const I mean, const I stddev) const {
        const auto a = random.normal<Pos<N>, I>(mean, stddev);
        const auto b = random.normal<Pos<N>, I>(mean, stddev);
        return Box<2>(a, b);
    }
};

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

Set<At<2, Edge<2, I64>>> view(List<Edge<2, I64>> &list, const Pos<2> &offset) {
    Set<At<2, Edge<2, I64>>> set;
    for (Edge<2, I64> &item : list) {
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
    List<Edge<2, I64>> edges = box->bbox().edges();

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