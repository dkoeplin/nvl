#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "nvl/geo/RTree.h"
#include "nvl/geo/Tuple.h"
#include "nvl/geo/Util.h"
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
    pure bool operator==(const IDs &rhs) const {
        return origin == rhs.origin && grid_size == rhs.grid_size && ids == rhs.ids;
    }
    pure bool operator!=(const IDs &rhs) const { return !(*this == rhs); }

    Pos<2> origin;
    I64 grid_size;
    Set<U64> ids;
};

std::ostream &operator<<(std::ostream &os, const IDs &ids) {
    return os << "{" << ids.origin << "+/-" << ids.grid_size << ", " << ids.ids << "}";
}

/// Returns a Map from the lowest level volume buckets to all IDs contained in that bucket.
template <U64 N, typename Item, typename ItemRef, U64 kMaxEntries>
pure List<IDs> collect_ids(RTree<N, Item, ItemRef, kMaxEntries> &tree)
    requires HasID<Item>
{
    using Node = RTree<N, Item, ItemRef, kMaxEntries>::Node;
    List<IDs> list;
    tree.preorder_walk_nodes([&](const Node *node) {
        Set<U64> ids;
        for (const ItemRef &item : node->list) {
            ids.insert(item->id());
        }
        if (!ids.empty()) {
            list.emplace_back(node->origin, node->grid_size, ids);
        }
        return WalkResult::kRecurse;
    });
    return list;
}

TEST(TestRTree, create) {
    RTree<2, LabeledBox> tree;
    tree.insert({0, {{0, 5}, {5, 10}}});
    EXPECT_EQ(tree.size(), 1);
    EXPECT_EQ(tree.nodes(), 1); // RTree is the root, no children are created yet
    EXPECT_EQ(tree.grid_size, 16);
    tree.dump();
}

TEST(TestRTree, create_high_loc) {
    RTree<2, LabeledBox> tree;
    tree.emplace(0, Box<2>({10000, 10000}, {10005, 10005}));
    EXPECT_EQ(tree.size(), 1);
    EXPECT_EQ(tree.nodes(), 1);
    EXPECT_EQ(tree.grid_size, 16384);
    tree.dump();
}

TEST(TestRTree, grow_root) {
    RTree<2, LabeledBox> tree;
    tree.emplace(0, Box<2>({5, 5}, {100, 100}));
    EXPECT_EQ(tree.size(), 1);
    EXPECT_EQ(tree.nodes(), 1);
    EXPECT_EQ(tree.grid_size, 128);

    tree.emplace(1, Box<2>({3000, 1200}, {3014, 1215}));
    EXPECT_EQ(tree.size(), 2);
    EXPECT_EQ(tree.nodes(), 1);
    EXPECT_EQ(tree.grid_size, 4096);

    EXPECT_THAT(collect_ids(tree), UnorderedElementsAre(IDs{.origin = {0, 0}, .grid_size = 4096, .ids = {0, 1}}));
    tree.dump();
}

TEST(TestRTree, subdivide) {
    RTree<2, LabeledBox, Ref<LabeledBox>, /*max_entries*/ 2> tree;
    const auto b0 = tree.emplace(0, Box<2>({0, 5}, {12, 22}));
    const auto b1 = tree.emplace(1, Box<2>({10, 100}, {22, 122}));
    const auto b2 = tree.emplace(2, Box<2>({100, 200}, {202, 202}));

    EXPECT_EQ(tree.size(), 3);  // Number of values
    EXPECT_EQ(tree.nodes(), 5); // Number of nodes
    EXPECT_THAT(collect_ids(tree), UnorderedElementsAre(IDs{.origin = {192, 192}, .grid_size = 64, .ids = {2}},
                                                        IDs{.origin = {64, 192}, .grid_size = 64, .ids = {2}},
                                                        IDs{.origin = {64, 64}, .grid_size = 64, .ids = {0, 1}}));

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
                UnorderedElementsAre(IDs{.origin = {1536, 512}, .grid_size = 512, .ids = {1}},
                                     IDs{.origin = {512, 1536}, .grid_size = 512, .ids = {2}},
                                     IDs{.origin = {768, 768}, .grid_size = 256, .ids = {0, 1, 2, 3, 4, 5, 6, 7, 8}},
                                     IDs{.origin = {256, 768}, .grid_size = 256, .ids = {1}},
                                     IDs{.origin = {768, 256}, .grid_size = 256, .ids = {8, 9}}));
}

TEST(TestRTree, negative_buckets) {
    RTree<2, LabeledBox, Ref<LabeledBox>, 1> tree;
    tree.insert({0, Box<2>({0, 882}, {1512, 982})});
    tree.insert({1, Box<2>({346, -398}, {666, -202})});

    EXPECT_THAT(collect_ids(tree), UnorderedElementsAre(IDs{.origin = {1024, 1024}, .grid_size = 1024, .ids = {0}},
                                                        IDs{.origin = {1024, -1024}, .grid_size = 1024, .ids = {1}}));
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
    for (const Box<3> &box : size.volumes(/*step*/ 100'000)) {
        auto items = tree[box];
        ASSERT_EQ(items.size(), 1);
        EXPECT_EQ(items.begin()->raw(), box);
    }
    EXPECT_EQ(tree.size(), 400);
    EXPECT_EQ(tree.grid_size, 1 << nvl::bit_width(1'000'000));
}

// Current best is ~1.9us / call
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

// Current best is ~2us / call
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

// Current best is ~6.5us / call
TEST_F(FuzzComponents, components2d) {
    this->num_tests = 1E6;
    this->in[0] = Distribution::Uniform<I64>(100, 400);
    this->in[1] = Distribution::Uniform<I64>(5, 50);
    this->in[2] = Distribution::Uniform<I64>(0, 100);

    fuzz([](bool &result, const Pos<2> &shape0, const Pos<2> &shape1, const Pos<2> &loc) {
        const Box<2> box(Pos<2>::zero, shape0);
        const Box<2> rem(loc, loc + shape1);
        const List<Box<2>> diff = box.diff(rem);
        RTree<2, Box<2>> tree;
        tree.insert(diff);
        EXPECT_EQ(tree.components().size(), 1);
        result = true;
    });
}

TEST(TestRTree, components) {
    List<Box<1>> list;
    list.push_back(Box<1>{Pos<1>{436639876131680LL}, Pos<1>{436867778393909LL}});
    list.push_back(Box<1>{Pos<1>{197700960860637LL}, Pos<1>{198893061877807LL}});
    list.push_back(Box<1>{Pos<1>{255753644093984LL}, Pos<1>{256141049798710LL}});
    list.push_back(Box<1>{Pos<1>{2351348670325LL}, Pos<1>{2617031498964LL}});
    list.push_back(Box<1>{Pos<1>{132305455060243LL}, Pos<1>{136355961131250LL}});
    list.push_back(Box<1>{Pos<1>{435738105327068LL}, Pos<1>{436449599122322LL}});
    list.push_back(Box<1>{Pos<1>{103525263650762LL}, Pos<1>{109263601125388LL}});
    list.push_back(Box<1>{Pos<1>{325790710810736LL}, Pos<1>{327242721326767LL}});
    list.push_back(Box<1>{Pos<1>{14267065719571LL}, Pos<1>{19713754885182LL}});
    list.push_back(Box<1>{Pos<1>{193190914974227LL}, Pos<1>{194389809136272LL}});
    list.push_back(Box<1>{Pos<1>{293982406793751LL}, Pos<1>{295216301045007LL}});
    list.push_back(Box<1>{Pos<1>{323845023844190LL}, Pos<1>{329536560310240LL}});
    list.push_back(Box<1>{Pos<1>{292907892178176LL}, Pos<1>{294163809044608LL}});
    list.push_back(Box<1>{Pos<1>{441279431671536LL}, Pos<1>{441734947012176LL}});
    list.push_back(Box<1>{Pos<1>{195881464266764LL}, Pos<1>{197136077647966LL}});
    list.push_back(Box<1>{Pos<1>{438992222880514LL}, Pos<1>{439720188043394LL}});
    list.push_back(Box<1>{Pos<1>{1138886722630LL}, Pos<1>{1854190426884LL}});
    list.push_back(Box<1>{Pos<1>{2351348670325LL}, Pos<1>{2617031498964LL}});
    list.push_back(Box<1>{Pos<1>{260402819310713LL}, Pos<1>{260801313119542LL}});
    list.push_back(Box<1>{Pos<1>{254684163193254LL}, Pos<1>{254912091501407LL}});
    list.push_back(Box<1>{Pos<1>{320106355013024LL}, Pos<1>{320106355013025LL}});
    list.push_back(Box<1>{Pos<1>{302542078244300LL}, Pos<1>{305345714739585LL}});
    list.push_back(Box<1>{Pos<1>{298727251504138LL}, Pos<1>{300461809160697LL}});
    list.push_back(Box<1>{Pos<1>{396167498820829LL}, Pos<1>{398717779535762LL}});
    list.push_back(Box<1>{Pos<1>{4400855877487LL}, Pos<1>{4868742298199LL}});
    list.push_back(Box<1>{Pos<1>{257130691987536LL}, Pos<1>{257951515966144LL}});
    list.push_back(Box<1>{Pos<1>{541881978152383LL}, Pos<1>{541881978152384LL}});
    list.push_back(Box<1>{Pos<1>{196610234096194LL}, Pos<1>{198190244156018LL}});
    list.push_back(Box<1>{Pos<1>{349383978844235LL}, Pos<1>{349762616770402LL}});
    list.push_back(Box<1>{Pos<1>{183960576643697LL}, Pos<1>{184978726668744LL}});
    list.push_back(Box<1>{Pos<1>{444969405647581LL}, Pos<1>{446718802519497LL}});
    list.push_back(Box<1>{Pos<1>{61304715018545LL}, Pos<1>{66159151881094LL}});
    list.push_back(Box<1>{Pos<1>{81520617554440LL}, Pos<1>{87934729706191LL}});
    list.push_back(Box<1>{Pos<1>{259834369549045LL}, Pos<1>{260402819310714LL}});
    list.push_back(Box<1>{Pos<1>{434587230987565LL}, Pos<1>{435466102099161LL}});
    list.push_back(Box<1>{Pos<1>{475435666013023LL}, Pos<1>{481706905967205LL}});
    list.push_back(Box<1>{Pos<1>{533947161016854LL}, Pos<1>{541881978152384LL}});
    list.push_back(Box<1>{Pos<1>{514845169944552LL}, Pos<1>{518052420123996LL}});
    list.push_back(Box<1>{Pos<1>{416462519847165LL}, Pos<1>{420821553093954LL}});
    list.push_back(Box<1>{Pos<1>{396167498820829LL}, Pos<1>{398717779535762LL}});
    list.push_back(Box<1>{Pos<1>{466870584819758LL}, Pos<1>{468197356102743LL}});
    list.push_back(Box<1>{Pos<1>{377045912599205LL}, Pos<1>{379402019526562LL}});
    list.push_back(Box<1>{Pos<1>{121492005967889LL}, Pos<1>{128192960880609LL}});
    list.push_back(Box<1>{Pos<1>{165350397722588LL}, Pos<1>{166567604961684LL}});
    list.push_back(Box<1>{Pos<1>{523602216130118LL}, Pos<1>{528877477075964LL}});
    list.push_back(Box<1>{Pos<1>{486794930310861LL}, Pos<1>{490399985238636LL}});
    list.push_back(Box<1>{Pos<1>{346668568123798LL}, Pos<1>{347498421357502LL}});
    list.push_back(Box<1>{Pos<1>{263741759784930LL}, Pos<1>{270004591104065LL}});
    list.push_back(Box<1>{Pos<1>{453510559403827LL}, Pos<1>{453510559403828LL}});
    list.push_back(Box<1>{Pos<1>{255753644093984LL}, Pos<1>{255883541623863LL}});
    list.push_back(Box<1>{Pos<1>{237029632750300LL}, Pos<1>{240866198325709LL}});
    list.push_back(Box<1>{Pos<1>{66159151881095LL}, Pos<1>{67650364848460LL}});
    list.push_back(Box<1>{Pos<1>{494183422785944LL}, Pos<1>{500125695261258LL}});
    list.push_back(Box<1>{Pos<1>{345448231703112LL}, Pos<1>{345736097412169LL}});
    list.push_back(Box<1>{Pos<1>{299884465540750LL}, Pos<1>{301450036951234LL}});
    list.push_back(Box<1>{Pos<1>{342394384522054LL}, Pos<1>{342917428478174LL}});
    list.push_back(Box<1>{Pos<1>{342394384522054LL}, Pos<1>{343269756664440LL}});
    list.push_back(Box<1>{Pos<1>{439720188043393LL}, Pos<1>{439932277547992LL}});
    list.push_back(Box<1>{Pos<1>{199278226329158LL}, Pos<1>{200874723399667LL}});
    list.push_back(Box<1>{Pos<1>{156628599411913LL}, Pos<1>{156628599411914LL}});
    list.push_back(Box<1>{Pos<1>{114849046814540LL}, Pos<1>{118625964441382LL}});
    list.push_back(Box<1>{Pos<1>{348650279799842LL}, Pos<1>{349135926566983LL}});
    list.push_back(Box<1>{Pos<1>{251363096532313LL}, Pos<1>{251754519029659LL}});
    list.push_back(Box<1>{Pos<1>{2351348670325LL}, Pos<1>{3059454811611LL}});
    list.push_back(Box<1>{Pos<1>{121492005967889LL}, Pos<1>{128192960880609LL}});
    list.push_back(Box<1>{Pos<1>{215903109993819LL}, Pos<1>{220160520317359LL}});
    list.push_back(Box<1>{Pos<1>{4595724410262LL}, Pos<1>{4868742298199LL}});
    list.push_back(Box<1>{Pos<1>{11764600261815LL}, Pos<1>{16408688324352LL}});
    list.push_back(Box<1>{Pos<1>{305345714739585LL}, Pos<1>{309765283857436LL}});
    list.push_back(Box<1>{Pos<1>{343686692179376LL}, Pos<1>{343965801195005LL}});
    list.push_back(Box<1>{Pos<1>{198536528306933LL}, Pos<1>{199895172276410LL}});
    list.push_back(Box<1>{Pos<1>{290236924432701LL}, Pos<1>{290236924432702LL}});
    list.push_back(Box<1>{Pos<1>{448383547620704LL}, Pos<1>{449913008882161LL}});
    list.push_back(Box<1>{Pos<1>{382502598531196LL}, Pos<1>{390914491628141LL}});
    list.push_back(Box<1>{Pos<1>{405909420201677LL}, Pos<1>{410130541503466LL}});
    list.push_back(Box<1>{Pos<1>{183960576643697LL}, Pos<1>{188237935670009LL}});
    list.push_back(Box<1>{Pos<1>{51962013277697LL}, Pos<1>{53813300126586LL}});
    list.push_back(Box<1>{Pos<1>{434353281411495LL}, Pos<1>{435243524406102LL}});
    list.push_back(Box<1>{Pos<1>{223464595521448LL}, Pos<1>{223464595521449LL}});
    list.push_back(Box<1>{Pos<1>{453510559403828LL}, Pos<1>{459559466120966LL}});
    list.push_back(Box<1>{Pos<1>{252153126561253LL}, Pos<1>{252392000276326LL}});
    list.push_back(Box<1>{Pos<1>{191071678578795LL}, Pos<1>{192431819097040LL}});
    list.push_back(Box<1>{Pos<1>{362692344706710LL}, Pos<1>{370654124816168LL}});
    list.push_back(Box<1>{Pos<1>{296854251448735LL}, Pos<1>{298173173031699LL}});
    list.push_back(Box<1>{Pos<1>{443669236357665LL}, Pos<1>{445234551662342LL}});
    list.push_back(Box<1>{Pos<1>{354976739042432LL}, Pos<1>{359653081133688LL}});
    list.push_back(Box<1>{Pos<1>{3059454811610LL}, Pos<1>{3402903612048LL}});
    list.push_back(Box<1>{Pos<1>{437155425700716LL}, Pos<1>{437395474195480LL}});
    list.push_back(Box<1>{Pos<1>{553779369768552LL}, Pos<1>{558816033078842LL}});
    list.push_back(Box<1>{Pos<1>{1854190426883LL}, Pos<1>{2617031498964LL}});
    list.push_back(Box<1>{Pos<1>{202597802194011LL}, Pos<1>{206543173679957LL}});
    list.push_back(Box<1>{Pos<1>{1138886722630LL}, Pos<1>{1854190426884LL}});
    list.push_back(Box<1>{Pos<1>{192264299244619LL}, Pos<1>{193376425428412LL}});
    list.push_back(Box<1>{Pos<1>{349135926566982LL}, Pos<1>{349762616770402LL}});
    list.push_back(Box<1>{Pos<1>{255522055688549LL}, Pos<1>{255883541623863LL}});
    list.push_back(Box<1>{Pos<1>{257951515966143LL}, Pos<1>{258174481575673LL}});
    list.push_back(Box<1>{Pos<1>{255883541623862LL}, Pos<1>{256689490174133LL}});
    list.push_back(Box<1>{Pos<1>{387931485549597LL}, Pos<1>{390914491628141LL}});
    list.push_back(Box<1>{Pos<1>{6447002120018LL}, Pos<1>{6846703895095LL}});
    list.push_back(Box<1>{Pos<1>{507044218634982LL}, Pos<1>{510418885916605LL}});
    list.push_back(Box<1>{Pos<1>{523602216130118LL}, Pos<1>{528877477075964LL}});
    list.push_back(Box<1>{Pos<1>{6846703895094LL}, Pos<1>{7184163411934LL}});
    list.push_back(Box<1>{Pos<1>{132305455060243LL}, Pos<1>{138392298598801LL}});
    list.push_back(Box<1>{Pos<1>{282461539896047LL}, Pos<1>{290236924432702LL}});
    list.push_back(Box<1>{Pos<1>{426299821128178LL}, Pos<1>{431250671557139LL}});
    list.push_back(Box<1>{Pos<1>{91615122525877LL}, Pos<1>{99193390510675LL}});
    list.push_back(Box<1>{Pos<1>{77643299597LL}, Pos<1>{926402139636LL}});
    list.push_back(Box<1>{Pos<1>{23129086310222LL}, Pos<1>{29305409989804LL}});
    list.push_back(Box<1>{Pos<1>{295643608940970LL}, Pos<1>{297332743231860LL}});
    list.push_back(Box<1>{Pos<1>{263741759784930LL}, Pos<1>{263741759784931LL}});
    list.push_back(Box<1>{Pos<1>{494183422785944LL}, Pos<1>{494183422785945LL}});
    list.push_back(Box<1>{Pos<1>{450219329929070LL}, Pos<1>{452081472445467LL}});
    list.push_back(Box<1>{Pos<1>{477672953090653LL}, Pos<1>{481706905967205LL}});
    list.push_back(Box<1>{Pos<1>{42025986713405LL}, Pos<1>{48342473949311LL}});
    list.push_back(Box<1>{Pos<1>{7741071773556LL}, Pos<1>{8327653006671LL}});
    list.push_back(Box<1>{Pos<1>{214028105426882LL}, Pos<1>{215903109993819LL}});
    list.push_back(Box<1>{Pos<1>{254527097937093LL}, Pos<1>{254684163193255LL}});
    list.push_back(Box<1>{Pos<1>{291709376480177LL}, Pos<1>{293396513721434LL}});
    list.push_back(Box<1>{Pos<1>{112530102990519LL}, Pos<1>{118625964441382LL}});
    list.push_back(Box<1>{Pos<1>{242070259011311LL}, Pos<1>{249530233518232LL}});
    list.push_back(Box<1>{Pos<1>{297760250261113LL}, Pos<1>{299278734305542LL}});
    list.push_back(Box<1>{Pos<1>{416462519847164LL}, Pos<1>{416462519847165LL}});
    list.push_back(Box<1>{Pos<1>{56632535322167LL}, Pos<1>{58731225107502LL}});
    list.push_back(Box<1>{Pos<1>{334482514270603LL}, Pos<1>{339477569119551LL}});
    list.push_back(Box<1>{Pos<1>{81520617554439LL}, Pos<1>{81520617554440LL}});
    list.push_back(Box<1>{Pos<1>{58203914594292LL}, Pos<1>{59974261998035LL}});
    list.push_back(Box<1>{Pos<1>{349383978844235LL}, Pos<1>{349762616770402LL}});
    list.push_back(Box<1>{Pos<1>{2135269863759LL}, Pos<1>{2859607276209LL}});
    list.push_back(Box<1>{Pos<1>{8327653006670LL}, Pos<1>{9225102488934LL}});
    list.push_back(Box<1>{Pos<1>{55390560834705LL}, Pos<1>{57384780686929LL}});
    list.push_back(Box<1>{Pos<1>{34160582965213LL}, Pos<1>{38080109850519LL}});
    list.push_back(Box<1>{Pos<1>{433924147210519LL}, Pos<1>{434104886793943LL}});
    list.push_back(Box<1>{Pos<1>{514845169944552LL}, Pos<1>{521116262869678LL}});
    list.push_back(Box<1>{Pos<1>{166567604961684LL}, Pos<1>{166567604961685LL}});
    list.push_back(Box<1>{Pos<1>{6004431119448LL}, Pos<1>{6179579759734LL}});
    list.push_back(Box<1>{Pos<1>{442544600864973LL}, Pos<1>{444396613753446LL}});
    list.push_back(Box<1>{Pos<1>{314203428586513LL}, Pos<1>{320106355013024LL}});
    list.push_back(Box<1>{Pos<1>{141827641940356LL}, Pos<1>{148641129957013LL}});
    list.push_back(Box<1>{Pos<1>{445992312922664LL}, Pos<1>{447520263642759LL}});
    list.push_back(Box<1>{Pos<1>{54576882197287LL}, Pos<1>{56478874126452LL}});
    list.push_back(Box<1>{Pos<1>{272180081867612LL}, Pos<1>{276826775088462LL}});
    list.push_back(Box<1>{Pos<1>{175555988602786LL}, Pos<1>{176659954496398LL}});
    list.push_back(Box<1>{Pos<1>{483295065685033LL}, Pos<1>{487816296558128LL}});
    list.push_back(Box<1>{Pos<1>{242070259011311LL}, Pos<1>{249530233518232LL}});
    list.push_back(Box<1>{Pos<1>{5051996523582LL}, Pos<1>{6004431119449LL}});
    list.push_back(Box<1>{Pos<1>{1854190426883LL}, Pos<1>{2135269863760LL}});
    list.push_back(Box<1>{Pos<1>{194141831401666LL}, Pos<1>{195323930008517LL}});
    list.push_back(Box<1>{Pos<1>{449106805556747LL}, Pos<1>{451175636597920LL}});
    list.push_back(Box<1>{Pos<1>{233591212883937LL}, Pos<1>{240866198325709LL}});
    list.push_back(Box<1>{Pos<1>{223464595521449LL}, Pos<1>{230205054673551LL}});
    list.push_back(Box<1>{Pos<1>{463724193285528LL}, Pos<1>{470600784408068LL}});
    list.push_back(Box<1>{Pos<1>{348650279799842LL}, Pos<1>{348863902351622LL}});
    list.push_back(Box<1>{Pos<1>{352280675243247LL}, Pos<1>{358773687775855LL}});
    list.push_back(Box<1>{Pos<1>{48342473949310LL}, Pos<1>{48342473949311LL}});
    list.push_back(Box<1>{Pos<1>{431250671557139LL}, Pos<1>{431250671557140LL}});
    list.push_back(Box<1>{Pos<1>{447099768779328LL}, Pos<1>{448528894445877LL}});
    list.push_back(Box<1>{Pos<1>{349383978844235LL}, Pos<1>{350138800479176LL}});
    list.push_back(Box<1>{Pos<1>{103525263650762LL}, Pos<1>{103525263650763LL}});
    list.push_back(Box<1>{Pos<1>{350917040198065LL}, Pos<1>{351121872602029LL}});
    list.push_back(Box<1>{Pos<1>{155000750859284LL}, Pos<1>{156628599411914LL}});
    list.push_back(Box<1>{Pos<1>{50733245894011LL}, Pos<1>{52622686819435LL}});
    list.push_back(Box<1>{Pos<1>{171762399865226LL}, Pos<1>{180370699099142LL}});
    list.push_back(Box<1>{Pos<1>{438627291261910LL}, Pos<1>{439468151658995LL}});
    list.push_back(Box<1>{Pos<1>{294843638106269LL}, Pos<1>{296309821215655LL}});
    list.push_back(Box<1>{Pos<1>{73049437165533LL}, Pos<1>{77659104685676LL}});
    list.push_back(Box<1>{Pos<1>{410879657749LL}, Pos<1>{1138886722631LL}});
    list.push_back(Box<1>{Pos<1>{31094751119447LL}, Pos<1>{34160582965213LL}});
    list.push_back(Box<1>{Pos<1>{433195393462687LL}, Pos<1>{433650918405707LL}});
    list.push_back(Box<1>{Pos<1>{433924147210519LL}, Pos<1>{434353281411496LL}});
    list.push_back(Box<1>{Pos<1>{256141049798709LL}, Pos<1>{257023728478503LL}});
    list.push_back(Box<1>{Pos<1>{374512141167076LL}, Pos<1>{377045912599204LL}});
    list.push_back(Box<1>{Pos<1>{94512132119711LL}, Pos<1>{96626145043645LL}});
    list.push_back(Box<1>{Pos<1>{350028006993020LL}, Pos<1>{350408432945328LL}});
    list.push_back(Box<1>{Pos<1>{405909420201677LL}, Pos<1>{405909420201678LL}});
    list.push_back(Box<1>{Pos<1>{558816033078842LL}, Pos<1>{561397097410103LL}});
    list.push_back(Box<1>{Pos<1>{544924158791414LL}, Pos<1>{547465000068999LL}});
    list.push_back(Box<1>{Pos<1>{194756336340510LL}, Pos<1>{196079385685653LL}});
    list.push_back(Box<1>{Pos<1>{206543173679958LL}, Pos<1>{208800572147214LL}});
    list.push_back(Box<1>{Pos<1>{253635004041523LL}, Pos<1>{253905636443612LL}});
    list.push_back(Box<1>{Pos<1>{348863902351621LL}, Pos<1>{349762616770402LL}});
    list.push_back(Box<1>{Pos<1>{4868742298198LL}, Pos<1>{5051996523583LL}});
    list.push_back(Box<1>{Pos<1>{52847898151490LL}, Pos<1>{55115334701039LL}});
    list.push_back(Box<1>{Pos<1>{274411757710907LL}, Pos<1>{279797390154033LL}});
    list.push_back(Box<1>{Pos<1>{29305409989804LL}, Pos<1>{29305409989805LL}});
    list.push_back(Box<1>{Pos<1>{334482514270603LL}, Pos<1>{336538665223903LL}});
    list.push_back(Box<1>{Pos<1>{547465000069000LL}, Pos<1>{551608311243322LL}});

    RTree<1, Box<1>> tree;
    for (auto box : list) {
        tree.insert(box);
    }
    List<Box<1>> components;
    for (auto box : list) {
        bool found_component = false;
        for (auto &component : components) {
            if (box.overlaps(component)) {
                component = bounding_box(box, component);
                found_component = true;
                break;
            }
        }
        if (!found_component) {
            components.push_back(box);
        }
    }
    auto tree_components = tree.components();
    List<Box<1>> tree_boxes;
    for (auto component : tree_components) {
        auto box = bounding_box<1, I64, Box<1>>(component.values());
        tree_boxes.push_back(box);
    }
    for (U64 i = 0; i < tree_boxes.size(); ++i) {
        for (U64 j = i + 1; j < tree_boxes.size(); ++j) {
            ASSERT(!tree_boxes[i].overlaps(tree_boxes[j]),
                   "Overlapping components: " << tree_boxes[i] << " and " << tree_boxes[j]);
        }
    }
}

} // namespace
