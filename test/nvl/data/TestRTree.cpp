#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "nvl/data/RTree.h"
#include "nvl/geo/Box.h"
#include "nvl/geo/Pos.h"

namespace {

using testing::ElementsAre;
using testing::IsEmpty;
using testing::UnorderedElementsAre;

using nvl::Box;
using nvl::List;
using nvl::Map;
using nvl::Pos;
using nvl::RTree;
using nvl::Set;

struct LabeledBox {
    LabeledBox() = default;
    LabeledBox(const U64 id, const Box<2> &box) : id(id), box(box) {}
    pure bool operator==(const LabeledBox &rhs) const { return id == rhs.id && box == rhs.box; }
    pure bool operator!=(const LabeledBox &rhs) const { return !(*this == rhs); }
    LabeledBox operator+(const Pos<2> &offset) const { return {id, box + offset}; }
    U64 id;
    Box<2> box;
};

std::ostream &operator<<(std::ostream &os, const LabeledBox &v) { return os << "BOX(" << v.id << ": " << v.box << ")"; }

TEST(TestRTree, create) {
    RTree<2, LabeledBox> tree;
    tree.insert({0, {{0, 5}, {5, 10}}});
    EXPECT_EQ(tree.size(), 1);
    EXPECT_EQ(tree.nodes(), 1);
}

TEST(TestRTree, divide) {
    RTree<2, LabeledBox> tree;
    const LabeledBox b0{0, {{5, 5}, {100, 100}}};
    const LabeledBox b1{1, {{3000, 1200}, {3014, 1215}}};
    tree.insert(b0);
    tree.insert(b1);
    EXPECT_EQ(tree.size(), 2);
    EXPECT_EQ(tree.nodes(), 1);
    const Map<Box<2>, Set<U64>> expected{{Box<2>({0, 0}, {1023, 1023}), Set<U64>{0}},
                                         {Box<2>({2048, 1024}, {3071, 2047}), Set<U64>{1}}};
    EXPECT_EQ(tree.testing().collect_ids(), expected);
}

TEST(TestRTree, subdivide) {
    RTree<2, LabeledBox, /*max_entries*/ 2> tree;
    const LabeledBox b0{0, {{0, 5}, {10, 20}}};
    const LabeledBox b1{1, {{10, 100}, {20, 120}}};
    const LabeledBox b2{2, {{100, 200}, {200, 200}}};
    tree.insert(b0);
    tree.insert(b1);
    tree.insert(b2);

    EXPECT_EQ(tree.size(), 3);  // Number of values
    EXPECT_EQ(tree.nodes(), 4); // Number of nodes

    const Map<Box<2>, Set<U64>> expected{{Box<2>({0, 0}, {127, 127}), Set<U64>{0, 1}},
                                         {Box<2>({0, 128}, {127, 255}), Set<U64>{2}},
                                         {Box<2>({128, 128}, {255, 255}), Set<U64>{2}}};
    EXPECT_EQ(tree.testing().collect_ids(), expected);

    // Check that we find all values when iterating over the bounding box, but each value is returned exactly once.
    const auto range = tree[tree.bounds()];
    const List<LabeledBox> elements(range.begin(), range.end());
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
        ids.insert(box.id);
    }
    for (const auto &[id, box] : tree.unordered()) {
        if (box.box.overlaps(range)) {
            EXPECT_TRUE(ids.contains(id));
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

    RTree<2, LabeledBox, /*max_entries*/ 9> tree;
    for (auto [id, x] : box.unordered()) {
        tree.insert({id, x});
    }

    const Map<Box<2>, Set<U64>> expected{{Box<2>({1024, 0}, {2047, 1023}), Set<U64>{1}},
                                         {Box<2>({0, 1024}, {1023, 2047}), Set<U64>{2}},
                                         {Box<2>({512, 512}, {1023, 1023}), Set<U64>{0, 1, 2, 3, 4, 5, 6, 7, 8}},
                                         {Box<2>({0, 512}, {511, 1023}), Set<U64>{1}},
                                         {Box<2>({512, 0}, {1023, 511}), Set<U64>{8, 9}}};
    EXPECT_EQ(tree.testing().collect_ids(), expected);
}

TEST(TestRTree, negative_buckets) {
    RTree<2, LabeledBox> tree;
    tree.insert({0, Box<2>({0, 882}, {1512, 982})});
    tree.insert({1, Box<2>({346, -398}, {666, -202})});

    const Map<Box<2>, Set<U64>> expected{{Box<2>({0, -1024}, {1023, -1}), Set<U64>{1}},
                                         {Box<2>({0, 0}, {1023, 1023}), Set<U64>{0}},
                                         {Box<2>({1024, 0}, {2047, 1023}), Set<U64>{0}}};
    EXPECT_EQ(tree.testing().collect_ids(), expected);
}

TEST(TestRTree, fetch) {
    RTree<2, LabeledBox> tree;
    const LabeledBox a{1, {{0, 882}, {1512, 982}}};
    const LabeledBox b{2, {{346, -398}, {666, -202}}};
    tree.insert(a);
    tree.insert(b);

    const auto range0 = tree[{{0, -300}, {1024, 1000}}];
    const auto range1 = tree[{{0, 0}, {100, 100}}];
    const auto range2 = tree[{{0, 885}, {100, 886}}];

    EXPECT_THAT(range0.list(), UnorderedElementsAre(a, b));
    EXPECT_THAT(range1.list(), IsEmpty());
    EXPECT_THAT(range2.list(), UnorderedElementsAre(a));
}

} // namespace
