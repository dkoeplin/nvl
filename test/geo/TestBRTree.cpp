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

using nvl::Box;
using nvl::BRTree;
using nvl::Edge;
using nvl::List;
using nvl::Pos;
using nvl::Rel;
using nvl::Set;
using nvl::test::LabeledBox;

Set<Edge<2, I64>> unwrap(const nvl::Range<Rel<Edge<2, I64>>> &list) {
    Set<Edge<2, I64>> result;
    for (const Rel<Edge<2, I64>> &entry : list) {
        result.insert(*entry);
    }
    return result;
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

    const auto list = tree[{0, 0}];
    EXPECT_THAT(list, UnorderedElementsAre(box));
}

TEST(TestBRTree, move) {
    BRTree<2, LabeledBox> tree;
    const auto box = tree.emplace(1, Box<2>({0, 0}, {32, 32}));
    tree.loc = {500, 500};

    const auto list0 = tree[Pos<2>::zero];
    EXPECT_THAT(list0, IsEmpty());

    const auto list1 = tree[tree.loc];
    EXPECT_THAT(list1, UnorderedElementsAre(box));
}

TEST(TestBRTree, edges) {
    using Edge = BRTree<2, LabeledBox>::Edge;

    BRTree<2, LabeledBox> tree;
    const auto box = tree.emplace(1, Box<2>({0, 0}, {32, 32}));
    const Set<Edge> box_edges(box->bbox().edges().range());

    EXPECT_EQ(tree.edge_rtree().size(), 4);

    const Set<Edge> edges0 = unwrap(tree.edges());
    EXPECT_EQ(edges0, box_edges);
}

} // namespace
