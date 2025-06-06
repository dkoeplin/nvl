#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "nvl/geo/RTree.h"
#include "nvl/geo/Tuple.h"
#include "nvl/geo/Volume.h"
#include "nvl/math/Distribution.h"
#include "nvl/math/Random.h"
#include "nvl/test/Fuzzing.h"
#include "nvl/test/LabeledBox.h"

namespace {

using testing::IsEmpty;
using testing::UnorderedElementsAre;

using nvl::Box;
using nvl::Distribution;
using nvl::Line;
using nvl::List;
using nvl::Map;
using nvl::Pos;
using nvl::Ref;
using nvl::RTree;
using nvl::Set;
using nvl::Vec;
using nvl::WalkResult;
using nvl::test::LabeledBox;

template <typename T>
concept HasID = requires(T a) { a.id(); };

struct IDs {
    pure bool operator==(const IDs &rhs) const { return box == rhs.box && ids == rhs.ids; }
    pure bool operator!=(const IDs &rhs) const { return !(*this == rhs); }

    Box<2> box;
    Set<U64> ids;
};

std::ostream &operator<<(std::ostream &os, const IDs &ids) { return os << "{" << ids.box << ", " << ids.ids << "}"; }

/// Returns a Map from the lowest level volume buckets to all IDs contained in that bucket.
template <U64 N, typename Item, typename ItemRef, U64 kMaxEntries>
pure List<IDs> collect_ids(RTree<N, Item, ItemRef, kMaxEntries> &tree)
    requires HasID<Item>
{
    using Node = typename RTree<N, Item, ItemRef, kMaxEntries>::Node;
    List<IDs> list;
    tree.preorder_walk_nodes([&](Node *node) {
        Set<U64> ids;
        for (const auto &item : node->list) {
            ids.insert(item->id());
        }
        list.emplace_back(node->box(), ids);
        return WalkResult::kRecurse;
    });
    return list;
}

TEST(TestRTree, create) {
    RTree<2, LabeledBox> tree;
    tree.insert({0, {{0, 5}, {5, 10}}});
    EXPECT_EQ(tree.size(), 1);
    EXPECT_EQ(tree.nodes(), 1);
    tree.dump();
}

TEST(TestRTree, create_high_loc) {
    RTree<2, LabeledBox> tree;
    tree.emplace(0, Box<2>({10000, 10000}, {10005, 10005}));
    EXPECT_EQ(tree.size(), 1);
    EXPECT_EQ(tree.nodes(), 1);
    tree.dump();
}

TEST(TestRTree, divide) {
    RTree<2, LabeledBox> tree;
    tree.emplace(0, Box<2>({5, 5}, {100, 100}));
    tree.emplace(1, Box<2>({3000, 1200}, {3014, 1215}));
    EXPECT_EQ(tree.size(), 2);
    EXPECT_EQ(tree.nodes(), 2);

    EXPECT_THAT(collect_ids(tree), UnorderedElementsAre(IDs{.box = {{0, 0}, {1024, 1024}}, .ids = {0}},
                                                        IDs{.box = {{2048, 1024}, {3072, 2048}}, .ids = {1}}));
    tree.dump();
}

TEST(TestRTree, subdivide) {
    RTree<2, LabeledBox, Ref<LabeledBox>, /*max_entries*/ 2> tree;
    const auto b0 = tree.emplace(0, Box<2>({0, 5}, {12, 22}));
    const auto b1 = tree.emplace(1, Box<2>({10, 100}, {22, 122}));
    const auto b2 = tree.emplace(2, Box<2>({100, 200}, {202, 202}));

    EXPECT_EQ(tree.size(), 3);  // Number of values
    EXPECT_EQ(tree.nodes(), 4); // Number of nodes

    EXPECT_THAT(collect_ids(tree), UnorderedElementsAre(IDs{.box = {{0, 0}, {128, 128}}, .ids = {0, 1}},
                                                        IDs{.box = {{0, 128}, {128, 256}}, .ids = {2}},
                                                        IDs{.box = {{128, 128}, {256, 256}}, .ids = {2}}));

    // Check that we find all values when iterating over the bounding box, but each value is returned exactly once.
    const Set<Ref<LabeledBox>> elements = tree[tree.bbox()];
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
    box[0] = Box<2>({512, 512}, {515, 515});
    box[1] = Box<2>({0, 882}, {1513, 983}) + Pos<2>{0, 0};
    box[2] = Box<2>({614, 762}, {763, 882}) + Pos<2>(0, 253);
    box[3] = Box<2>({594, 701}, {716, 762}) + Pos<2>(0, 232);
    box[4] = Box<2>({620, 641}, {685, 701}) + Pos<2>(0, 183);
    box[5] = Box<2>({616, 592}, {687, 641}) + Pos<2>(0, 137);
    box[6] = Box<2>({603, 536}, {681, 592}) + Pos<2>(0, 132);
    box[7] = Box<2>({582, 474}, {663, 536}) + Pos<2>(0, 130);
    box[8] = Box<2>({615, 416}, {673, 474}) + Pos<2>(0, 82);
    box[9] = Box<2>({599, 375}, {648, 416}) + Pos<2>(0, 41);

    RTree<2, LabeledBox, Ref<LabeledBox>, /*max_entries*/ 9> tree;
    for (auto &[id, x] : box) {
        tree.insert({id, x});
    }

    EXPECT_THAT(collect_ids(tree),
                UnorderedElementsAre(IDs{.box = {{1024, 0}, {2048, 1024}}, .ids = {1}},
                                     IDs{.box = {{0, 1024}, {1024, 2048}}, .ids = {2}},
                                     IDs{.box = {{512, 512}, {1024, 1024}}, .ids = {0, 1, 2, 3, 4, 5, 6, 7, 8}},
                                     IDs{.box = {{0, 512}, {512, 1024}}, .ids = {1}},
                                     IDs{.box = {{512, 0}, {1024, 512}}, .ids = {8, 9}}));
}

TEST(TestRTree, negative_buckets) {
    RTree<2, LabeledBox> tree;
    tree.insert({0, Box<2>({0, 882}, {1512, 982})});
    tree.insert({1, Box<2>({346, -398}, {666, -202})});

    EXPECT_THAT(collect_ids(tree), UnorderedElementsAre(IDs{.box = {{0, -1024}, {1024, 0}}, .ids = {1}},
                                                        IDs{.box = {{0, 0}, {1024, 1024}}, .ids = {0}},
                                                        IDs{.box = {{1024, 0}, {2048, 1024}}, .ids = {0}}));
}

TEST(TestRTree, fetch) {
    RTree<2, LabeledBox> tree;
    const auto a = tree.emplace(1, Box<2>({0, 882}, {1512, 982}));
    const auto b = tree.emplace(2, Box<2>({346, -398}, {666, -202}));

    const Set<Ref<LabeledBox>> range0 = tree[{{0, -300}, {1024, 1000}}];
    const Set<Ref<LabeledBox>> range1 = tree[{{0, 0}, {100, 100}}];
    const Set<Ref<LabeledBox>> range2 = tree[{{0, 885}, {100, 886}}];

    EXPECT_THAT(range0, UnorderedElementsAre(a, b));
    EXPECT_THAT(range1, IsEmpty());
    EXPECT_THAT(range2, UnorderedElementsAre(a));
}

TEST(TestRTree, first_where) {
    constexpr Line<3> line{{528, 969, 410}, {528, 974, 510}};
    RTree<3, Box<3>> tree;
    tree.emplace<Box<3>>(Pos<3>{500, 950, 500}, Pos<3>{549, 999, 549});
    auto intersect = tree.first_where(line, [](const auto &intersect) { return intersect.dist; });
    ASSERT_TRUE(intersect.has_value());
    EXPECT_EQ(intersect->pt, Vec<3>(528, 973.5, 500));
}

TEST(TestRTree, move2d) {
    RTree<2, LabeledBox> tree;
    Ref<LabeledBox> box = tree.emplace(0, Box<2>{{-187, -448}, {1094, 983}});
    tree.dump();
    const Box<2> prev = box->bbox();
    box->moveto({9444, 5599});
    tree.move(box, prev);
    tree.dump();
}

TEST(TestRTree, empty_components) {
    RTree<2, LabeledBox> tree;
    EXPECT_THAT(tree.components(), IsEmpty());
}

TEST(TestRTree, components_individuals) {
    using Comp = Set<Ref<LabeledBox>>;
    RTree<2, LabeledBox> tree;
    const Ref<LabeledBox> a = tree.emplace(1, Box<2>({0, 0}, {10, 10}));
    const Ref<LabeledBox> b = tree.emplace(2, Box<2>({15, 15}, {20, 20}));
    const Ref<LabeledBox> c = tree.emplace(3, Box<2>({35, 35}, {40, 40}));
    EXPECT_THAT(tree.components(), UnorderedElementsAre(Comp{a}, Comp{b}, Comp{c}));
}

TEST(TestRTree, components_pairs) {
    using Comp = Set<Ref<LabeledBox>>;
    RTree<2, LabeledBox> tree;
    const Ref<LabeledBox> a = tree.emplace(1, Box<2>({0, 0}, {11, 11}));
    const Ref<LabeledBox> b = tree.emplace(2, Box<2>({11, 0}, {20, 11}));
    const Ref<LabeledBox> c = tree.emplace(3, Box<2>({35, 35}, {41, 41}));
    const Ref<LabeledBox> d = tree.emplace(4, Box<2>({38, 41}, {49, 101}));
    EXPECT_THAT(tree.components(), UnorderedElementsAre(Comp{a, b}, Comp{c, d}));
}

TEST(TestRTree, components_block) {
    using Comp = Set<Ref<LabeledBox>>;
    RTree<2, LabeledBox> tree;
    const Ref<LabeledBox> a = tree.emplace(1, Box<2>({0, 0}, {1, 1})); // 1
    const Ref<LabeledBox> b = tree.emplace(2, Box<2>({0, 1}, {1, 2})); // 2
    const Ref<LabeledBox> c = tree.emplace(3, Box<2>({0, 2}, {1, 3})); // 3
    const Ref<LabeledBox> d = tree.emplace(4, Box<2>({0, 3}, {1, 4})); // 4
    EXPECT_THAT(tree.components(), UnorderedElementsAre(Comp{a, b, c, d}));
}

TEST(TestRTree, components_complex) {
    constexpr Box<2> box{{817, 846}, {1134, 1105}};
    const List<Box<2>> rem{{{1100, 1005}, {1140, 1045}},
                           {{1063, 1005}, {1103, 1045}},
                           {{1024, 1005}, {1064, 1045}},
                           {{987, 1006}, {1027, 1046}}};
    List<Box<2>> diff = box.diff(rem);
    RTree<2, Box<2>> tree;
    tree.insert(diff.range());
    EXPECT_EQ(tree.components().size(), 1);
}

TEST(TestRTree, large_insertion) {
    RTree<3, Box<3>> tree;
    constexpr Box<3> size({-1'000'000, 0, -1'000'000}, {1'000'000, 1'000, 1'000'000});
    for (const Box<3> &box : size.volumes(/*step*/ 100'000)) {
        tree.emplace(box);
    }
    EXPECT_EQ(tree.root()->grid, 1 << nvl::ceil_log2(2'000'000));
    tree.dump();
}

TEST(TestRTree, fuzz_insertion) {
    constexpr I64 kNumTests = 1E3;
    RTree<2, Box<2>> tree;

    struct InsertFuzzer : nvl::test::Fuzzer<Ref<Box<2>>, Box<2>> {
        InsertFuzzer() : Fuzzer(0xDEADBEEF) {
            num_tests = kNumTests;
            in[0] = Distribution::Uniform<I64>(-100, 100);
        }
    } insert_fuzzer;

    insert_fuzzer.fuzz([&tree](Ref<Box<2>> &result, const Box<2> &in) { result = tree.insert(in); });

    EXPECT_EQ(tree.size(), kNumTests);
}

struct FuzzMove : nvl::test::FuzzingTestFixture<bool, Pos<2>, Pos<2>, Pos<2>> {
    FuzzMove() = default;
};

TEST_F(FuzzMove, move2d) {
    using nvl::Distribution;
    using nvl::Random;

    this->num_tests = 1E4;
    this->in[0] = Distribution::Uniform<I64>(500, 1500);
    this->in[1] = Distribution::Uniform<I64>(-1000, 0);
    this->in[2] = Distribution::Uniform<I64>(0, 10000);

    fuzz([](bool &passed, const Pos<2> &shape, const Pos<2> &loc, const Pos<2> &loc2) {
        RTree<2, LabeledBox> tree;
        const Box<2> original(loc, loc + shape);
        auto lbox = tree.emplace(0, original);
        lbox->moveto(loc2);
        tree.move(lbox, original);
        const Box<2> updated = lbox->bbox();
        for (Box<2> b : original.diff(updated)) {
            ASSERT_THAT(tree[b], IsEmpty());
        }
        for (Box<2> b : updated.diff(original)) {
            ASSERT_THAT(tree[b], UnorderedElementsAre(lbox)) //
                << "Expected to find box within " << b << " after moving: " << std::endl
                << "  Shape: " << shape << std::endl
                << "  Loc1:  " << loc << std::endl
                << "  Loc2:  " << loc2 << std::endl
                << "  Original: " << original << std::endl
                << "  Updated:  " << updated << std::endl;
        }
        passed = true;
    });
}

struct FuzzComponents : nvl::test::FuzzingTestFixture<bool, Pos<2>, Pos<2>, Pos<2>> {};

TEST_F(FuzzComponents, components2d) {
    this->num_tests = 1E3;
    this->in[0] = Distribution::Uniform<I64>(100, 400);
    this->in[1] = Distribution::Uniform<I64>(5, 50);
    this->in[2] = Distribution::Uniform<I64>(0, 100);

    fuzz([](bool &result, const Pos<2> &shape0, const Pos<2> &shape1, const Pos<2> &loc) {
        const Box<2> box(Pos<2>::zero, shape0);
        const Box<2> rem(loc, loc + shape1);
        const auto diff = box.diff(rem);
        RTree<2, Box<2>> tree;
        tree.insert(diff.range());
        EXPECT_EQ(tree.components().size(), 1);
        result = true;
    });
}

} // namespace
