#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <util/Fuzzing.h>

#include "nvl/geo/Box.h"
#include "nvl/geo/Pos.h"
#include "nvl/geo/RTree.h"
#include "util/LabeledBox.h"

namespace nvl {

template <U64 N>
struct nvl::RandomGen<Box<N>> {
    template <typename I>
    pure Box<N> uniform(Random &random, const I min, const I max) const {
        const auto a = random.uniform<Pos<N>, I>(min, max);
        const auto shape = random.uniform<Pos<N>, I>(0, 32);
        return Box(a, a + shape);
    }
    template <typename I>
    pure Box<N> normal(Random &random, const I mean, const I stddev) const {
        const auto a = random.normal<Pos<N>, I>(mean, stddev);
        const auto shape = random.uniform<Pos<N>, I>(0, 32);
        return Box(a, a + shape);
    }
};

} // namespace nvl

namespace {

using testing::IsEmpty;
using testing::UnorderedElementsAre;

using nvl::Box;
using nvl::Distribution;
using nvl::List;
using nvl::Map;
using nvl::Pos;
using nvl::Ref;
using nvl::RTree;
using nvl::Set;
using nvl::testing::LabeledBox;

template <typename T>
concept HasID = requires(T a) { a.id(); };

/// Returns a Map from the lowest level volume buckets to all IDs contained in that bucket.
template <U64 N, typename Item, typename ItemRef, U64 kMaxEntries>
pure Map<Box<N>, Set<U64>> collect_ids(RTree<N, Item, ItemRef, kMaxEntries> &tree)
    requires HasID<Item>
{
    using Node = typename RTree<N, Item, ItemRef>::Node;
    Map<Box<N>, Set<U64>> ids;
    for (const auto &[node, pos] : tree.entries_in(tree.get_bbox().value_or(Box<N>::kUnitBox))) {
        if (auto *entry = node->get(pos); entry && entry->kind == Node::Entry::kList) {
            const Box<N> box(pos, pos + node->grid - 1);
            for (const auto &item : entry->list) {
                ids[box].insert(item->id());
            }
        }
    }
    return ids;
}

TEST(TestRTree, create) {
    RTree<2, LabeledBox> tree;
    tree.insert({0, {{0, 5}, {5, 10}}});
    EXPECT_EQ(tree.size(), 1);
    EXPECT_EQ(tree.nodes(), 1);
}

TEST(TestRTree, divide) {
    RTree<2, LabeledBox> tree;
    tree.emplace(0, Box<2>({5, 5}, {100, 100}));
    tree.emplace(1, Box<2>({3000, 1200}, {3014, 1215}));
    EXPECT_EQ(tree.size(), 2);
    EXPECT_EQ(tree.nodes(), 1);

    const Map<Box<2>, Set<U64>> expected{{Box<2>({0, 0}, {1023, 1023}), Set<U64>{0}},
                                         {Box<2>({2048, 1024}, {3071, 2047}), Set<U64>{1}}};
    EXPECT_EQ(collect_ids(tree), expected);
}

TEST(TestRTree, subdivide) {
    RTree<2, LabeledBox, Ref<LabeledBox>, /*max_entries*/ 2> tree;
    const auto b0 = tree.emplace(0, Box<2>({0, 5}, {10, 20}));
    const auto b1 = tree.emplace(1, Box<2>({10, 100}, {20, 120}));
    const auto b2 = tree.emplace(2, Box<2>({100, 200}, {200, 200}));

    EXPECT_EQ(tree.size(), 3);  // Number of values
    EXPECT_EQ(tree.nodes(), 4); // Number of nodes

    const Map<Box<2>, Set<U64>> expected{{Box<2>({0, 0}, {127, 127}), Set<U64>{0, 1}},
                                         {Box<2>({0, 128}, {127, 255}), Set<U64>{2}},
                                         {Box<2>({128, 128}, {255, 255}), Set<U64>{2}}};
    EXPECT_EQ(collect_ids(tree), expected);

    // Check that we find all values when iterating over the bounding box, but each value is returned exactly once.
    const List<Ref<LabeledBox>> elements(tree[tree.bbox()]);
    EXPECT_THAT(elements, UnorderedElementsAre(b0, b1, b2));
}

TEST(TestRTree, bracket_operator) {
    RTree<2, LabeledBox> tree;
    tree.insert({1, {{0, 0}, {1512, 982}}});
    tree.insert({2, {{0, 263}, {812, 881}}});
    tree.insert({3, {{0, 223}, {824, 693}}});
    tree.insert({4, {{0, 254}, {750, 613}}});
    tree.insert({5, {{0, 175}, {801, 530}}});
    tree.insert({6, {{0, 130}, {736, 356}}});
    tree.insert({7, {{0, 107}, {702, 278}}});
    tree.insert({8, {{0, 72}, {753, 202}}});
    tree.insert({9, {{0, 373}, {433, 881}}});
    tree.insert({10, {{0, 218}, {483, 811}}});
    tree.insert({11, {{0, 284}, {1364, 881}}});
    tree.insert({12, {{0, 203}, {1347, 698}}});
    tree.insert({13, {{0, 45}, {1346, 539}}});

    Set<U64> ids;
    constexpr Box<2> range{{98, 526}, {99, 527}};
    for (const auto &box : tree[range]) {
        ids.insert(box->id());
    }
    for (const auto &box : tree) {
        if (box->bbox().overlaps(range)) {
            EXPECT_TRUE(ids.has(box->id()));
        }
    }
}

TEST(TestRTree, keep_buckets_after_subdivide) {
    Map<U64, Box<2>> box;
    box[0] = Box<2>({512, 512}, {514, 514});
    box[1] = Box<2>({0, 882}, {1512, 982}) + Pos<2>{0, 0};
    box[2] = Box<2>({614, 762}, {762, 881}) + Pos<2>(0, 253);
    box[3] = Box<2>({594, 701}, {715, 761}) + Pos<2>(0, 232);
    box[4] = Box<2>({620, 641}, {684, 700}) + Pos<2>(0, 183);
    box[5] = Box<2>({616, 592}, {686, 640}) + Pos<2>(0, 137);
    box[6] = Box<2>({603, 536}, {680, 591}) + Pos<2>(0, 132);
    box[7] = Box<2>({582, 474}, {662, 535}) + Pos<2>(0, 130);
    box[8] = Box<2>({615, 416}, {672, 473}) + Pos<2>(0, 82);
    box[9] = Box<2>({599, 375}, {647, 415}) + Pos<2>(0, 41);

    RTree<2, LabeledBox, Ref<LabeledBox>, /*max_entries*/ 9> tree;
    for (auto &[id, x] : box) {
        tree.insert({id, x});
    }

    const Map<Box<2>, Set<U64>> expected{{Box<2>({1024, 0}, {2047, 1023}), Set<U64>{1}},
                                         {Box<2>({0, 1024}, {1023, 2047}), Set<U64>{2}},
                                         {Box<2>({512, 512}, {1023, 1023}), Set<U64>{0, 1, 2, 3, 4, 5, 6, 7, 8}},
                                         {Box<2>({0, 512}, {511, 1023}), Set<U64>{1}},
                                         {Box<2>({512, 0}, {1023, 511}), Set<U64>{8, 9}}};
    EXPECT_EQ(collect_ids(tree), expected);
}

TEST(TestRTree, negative_buckets) {
    RTree<2, LabeledBox> tree;
    tree.insert({0, Box<2>({0, 882}, {1512, 982})});
    tree.insert({1, Box<2>({346, -398}, {666, -202})});

    const Map<Box<2>, Set<U64>> expected{{Box<2>({0, -1024}, {1023, -1}), Set<U64>{1}},
                                         {Box<2>({0, 0}, {1023, 1023}), Set<U64>{0}},
                                         {Box<2>({1024, 0}, {2047, 1023}), Set<U64>{0}}};
    EXPECT_EQ(collect_ids(tree), expected);
}

TEST(TestRTree, fetch) {
    RTree<2, LabeledBox> tree;
    const auto a = tree.emplace(1, Box<2>({0, 882}, {1512, 982}));
    const auto b = tree.emplace(2, Box<2>({346, -398}, {666, -202}));

    const List<Ref<LabeledBox>> range0(tree[{{0, -300}, {1024, 1000}}]);
    const List<Ref<LabeledBox>> range1(tree[{{0, 0}, {100, 100}}]);
    const List<Ref<LabeledBox>> range2(tree[{{0, 885}, {100, 886}}]);

    EXPECT_THAT(range0, UnorderedElementsAre(a, b));
    EXPECT_THAT(range1, IsEmpty());
    EXPECT_THAT(range2, UnorderedElementsAre(a));
}

TEST(TestRTree, empty_components) {
    RTree<2, LabeledBox> tree;
    EXPECT_THAT(tree.components(), IsEmpty());
}

TEST(TestRTree, components_individuals) {
    using Comp = RTree<2, LabeledBox>::Component;
    RTree<2, LabeledBox> tree;
    const Ref<LabeledBox> a = tree.emplace(1, Box<2>({0, 0}, {10, 10}));
    const Ref<LabeledBox> b = tree.emplace(2, Box<2>({15, 15}, {20, 20}));
    const Ref<LabeledBox> c = tree.emplace(3, Box<2>({35, 35}, {40, 40}));
    EXPECT_THAT(tree.components(), UnorderedElementsAre(Comp{a}, Comp{b}, Comp{c}));
}

TEST(TestRTree, components_pairs) {
    using Comp = RTree<2, LabeledBox>::Component;
    RTree<2, LabeledBox> tree;
    const Ref<LabeledBox> a = tree.emplace(1, Box<2>({0, 0}, {10, 10}));
    const Ref<LabeledBox> b = tree.emplace(2, Box<2>({11, 0}, {19, 10}));
    const Ref<LabeledBox> c = tree.emplace(3, Box<2>({35, 35}, {40, 40}));
    const Ref<LabeledBox> d = tree.emplace(4, Box<2>({38, 41}, {48, 100}));
    EXPECT_THAT(tree.components(), UnorderedElementsAre(Comp{a, b}, Comp{c, d}));
}

TEST(TestRTree, fuzz_insertion) {
    constexpr I64 kNumTests = 1E6;
    RTree<2, Box<2>> tree;

    struct InsertFuzzer : nvl::testing::Fuzzer<Ref<Box<2>>, Box<2>> {
        InsertFuzzer() : Fuzzer(0xDEADBEEF) {
            num_tests = kNumTests;
            in[0] = Distribution::Uniform<I64>(-100, 100);
        }
    } insert_fuzzer;

    insert_fuzzer.fuzz([&tree](Ref<Box<2>> &result, const Box<2> &in) { result = tree.insert(in); });

    EXPECT_EQ(tree.size(), kNumTests);
}

} // namespace
