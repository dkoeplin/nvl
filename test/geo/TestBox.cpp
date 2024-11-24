#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "nvl/geo/Box.h"
#include "nvl/geo/Pos.h"
#include "nvl/math/Distribution.h"
#include "nvl/math/Random.h"
#include "nvl/test/Fuzzing.h"

namespace nvl {

template <U64 N>
struct nvl::RandomGen<Box<N>> {
    template <typename I>
    pure Box<N> uniform(Random &random, const I min, const I max) const {
        const auto a = random.uniform<Pos<N>, I>(min, max);
        const auto b = random.uniform<Pos<N>, I>(min, max);
        return Box(a, b);
    }
    template <typename I>
    pure Box<N> normal(Random &random, const I mean, const I stddev) const {
        const auto a = random.normal<Pos<N>, I>(mean, stddev);
        const auto b = random.normal<Pos<N>, I>(mean, stddev);
        return Box(a, b);
    }
};

} // namespace nvl

namespace {
using testing::ElementsAre;
using testing::UnorderedElementsAre;

using nvl::Box;
using nvl::Dir;
using nvl::Edge;
using nvl::List;
using nvl::Pos;

TEST(TestBox, get) {
    constexpr Pos<2> a{6, 6};
    constexpr Pos<2> b{15, 6};
    const auto box = Box<2>::get(a, b);
    EXPECT_FALSE(box.has_value());
}

TEST(TestBox, shape) {
    constexpr Box<2> a({4, 5}, {33, 46});
    EXPECT_THAT(a.shape(), ElementsAre(29, 41));

    constexpr Box<2> b({2, 4}, {7, 11});
    EXPECT_THAT(b.shape(), ElementsAre(5, 7));

    constexpr Box<3> c({5, 8, 3}, {11, 14, 10});
    EXPECT_THAT(c.shape(), ElementsAre(6, 6, 7));
}

/*TEST(TestBox, mul) {
    constexpr Box<2> a{{2, 3}, {5, 6}};
    constexpr Pos<2> b{5, -1};
    EXPECT_EQ(a * b, Box<2>({10, -6}, {25, -3}));
    EXPECT_EQ(a * 2, Box<2>({4, 6}, {10, 12}));
}*/

TEST(TestBox, add) {
    constexpr Box<2> a({2, 3}, {7, 8});
    constexpr Pos<2> b{4, 2};
    EXPECT_EQ(a + b, Box<2>({6, 5}, {11, 10}));
    EXPECT_EQ(a + 5, Box<2>({7, 8}, {12, 13}));
}

TEST(TestBox, sub) {
    constexpr Box<2> a({2, 3}, {7, 8});
    constexpr Pos<2> b{4, 2};
    EXPECT_EQ(a - b, Box<2>({-2, 1}, {3, 6}));
    EXPECT_EQ(a - 4, Box<2>({-2, -1}, {3, 4}));
}

TEST(TestBox, pos_iter) {
    constexpr auto a = Box<2>({2, 4}, {5, 9});
    const auto iter0 = a.pos_iter();
    const List<Pos<2>> list0(iter0.begin(), iter0.end());
    const List<Pos<2>> expected0{{2, 4}, {2, 5}, {2, 6}, {2, 7}, {2, 8},  // {2, *}
                                 {3, 4}, {3, 5}, {3, 6}, {3, 7}, {3, 8},  // {3, *}
                                 {4, 4}, {4, 5}, {4, 6}, {4, 7}, {4, 8}}; // {4, *}
    EXPECT_EQ(list0, expected0);

    const auto iter1 = a.pos_iter(/*step*/ 2);
    const List<Pos<2>> list1(iter1.begin(), iter1.end());
    const List<Pos<2>> expected1{{2, 4}, {2, 6}, {2, 8}, {4, 4}, {4, 6}, {4, 8}};
    EXPECT_EQ(list1, expected1);

    const auto iter2 = a.pos_iter({1, 2});
    const List<Pos<2>> list2(iter2.begin(), iter2.end());
    const List<Pos<2>> expected2{{2, 4}, {2, 6}, {2, 8}, {3, 4}, {3, 6}, {3, 8}, {4, 4}, {4, 6}, {4, 8}};
    EXPECT_THAT(list2, expected2);

    EXPECT_DEATH({ std::cout << a.pos_iter({0, 2}); }, "Invalid iterator step size of 0");
    EXPECT_DEATH({ std::cout << a.pos_iter({-1, 2}); }, "TODO: Support negative step");
}

TEST(TestBox, box_iter) {
    constexpr Box<2> a({2, 2}, {7, 9}); // shape is 5x7

    const auto iter0 = a.box_iter({2, 2});
    const List<Box<2>> list0(iter0.begin(), iter0.end());
    const List<Box<2>> expected0{
        Box<2>({2, 2}, {4, 4}), // row 0:1, col 0:1
        Box<2>({2, 4}, {4, 6}), // row 0:1, col 2:3
        Box<2>({2, 6}, {4, 8}), // row 0:1, col 4:5
        Box<2>({4, 2}, {6, 4}), // row 2:3, col 0:1
        Box<2>({4, 4}, {6, 6}), // row 2:3, col 0:1
        Box<2>({4, 6}, {6, 8})  // row 2:3, col 0:1
    };
    EXPECT_EQ(list0, expected0);

    const auto iter1 = a.box_iter({1, 3});
    const List<Box<2>> list1(iter1.begin(), iter1.end());
    const List<Box<2>> expected1{
        Box<2>({2, 2}, {3, 5}), // row 0, col 0:2
        Box<2>({2, 5}, {3, 8}), // row 0, col 3:5
        Box<2>({3, 2}, {4, 5}), // row 1, col 0:2
        Box<2>({3, 5}, {4, 8}), // row 1, col 3:5
        Box<2>({4, 2}, {5, 5}), // row 2, col 0:2
        Box<2>({4, 5}, {5, 8}), // row 2, col 3:5
        Box<2>({5, 2}, {6, 5}), // row 3, col 0:2
        Box<2>({5, 5}, {6, 8}), // row 3, col 3:5
        Box<2>({6, 2}, {7, 5}), // row 2, col 0:2
        Box<2>({6, 5}, {7, 8}), // row 2, col 3:5
    };
    EXPECT_EQ(list1, expected1);

    EXPECT_DEATH({ std::cout << a.box_iter({0, 2}); }, "Invalid iterator shape size of 0");
    EXPECT_DEATH({ std::cout << a.box_iter({-1, 2}); }, "TODO: Support negative step");

    constexpr Box<2> b{{128, 128}, {256, 256}};
    const auto iter2 = b.box_iter(128);
    const List<Box<2>> list2(iter2.begin(), iter2.end());
    const List<Box<2>> expected2{Box<2>{{128, 128}, {256, 256}}};
    EXPECT_EQ(list2, expected2);
}

TEST(TestBox, clamp) {
    EXPECT_EQ(Box<2>({0, 0}, {512, 512}).clamp({1024, 1024}), Box<2>({0, 0}, {1024, 1024}));
    EXPECT_EQ(Box<2>({0, 0}, {1024, 1024}).clamp({1024, 1024}), Box<2>({0, 0}, {1024, 1024}));
    EXPECT_EQ(Box<2>({0, 0}, {1025, 1025}).clamp({1024, 1024}), Box<2>({0, 0}, {2048, 2048}));
    EXPECT_EQ(Box<2>({512, 512}, {1024, 1024}).clamp({1024, 1024}), Box<2>({0, 0}, {1024, 1024}));
    EXPECT_EQ(Box<2>({346, -398}, {666, -202}).clamp({1024, 1024}), Box<2>({0, -1024}, {1024, 0}));
    EXPECT_EQ(Box<2>({-100, 100}, {100, 300}).clamp({1024, 1024}), Box<2>({-1024, 0}, {1024, 1024}));
    EXPECT_EQ(Box<2>({128, 200}, {202, 202}).clamp({128, 128}), Box<2>({128, 128}, {256, 256}));
}

/*
 0 1 2 3 4 5 6
1
2      E E E
3    E X # # E
4    E # # # E
5    E # # Y E
6      E E E
*/
TEST(TestBox, edges) {
    constexpr Box<2> box({3, 3}, {5, 5});
    EXPECT_THAT(box.edges(), UnorderedElementsAre(Edge<2>(Dir::Neg, 0, Box<2>({2, 3}, {2, 5})),
                                                  Edge<2>(Dir::Pos, 0, Box<2>({6, 3}, {6, 5})),
                                                  Edge<2>(Dir::Neg, 1, Box<2>({3, 2}, {5, 2})),
                                                  Edge<2>(Dir::Pos, 1, Box<2>({3, 6}, {5, 6}))));
}

TEST(TestBox, overlaps) {
    constexpr Box<2> a({16, 5}, {16, 17});
    constexpr Box<2> b({8, 11}, {14, 16});
    EXPECT_FALSE(a.overlaps(b));
}

TEST(TestBox, intersect) {
    constexpr Box<2> a({16, 5}, {16, 17});
    constexpr Box<2> b({8, 11}, {14, 16});
    EXPECT_FALSE(a.intersect(b).has_value());

    constexpr Box<2> a0({100, 200}, {202, 202});
    constexpr Box<2> b0({128, 128}, {256, 256});
    EXPECT_EQ(a0.intersect(b0), Some(Box<2>({128, 200}, {202, 202})));
}

/*   0 1 2 3 4 << dim 0
   0
   1   X A B    ==> [X A B], [C], [D], [E F Y]
   2   C - D
   3   E F Y
   4
 */
TEST(TestBox, diff) {
    constexpr Box<2> box_a({1, 1}, {4, 4});
    constexpr Box<2> box_b({2, 2}, {3, 3});
    EXPECT_THAT(box_a.diff(box_b), UnorderedElementsAre(Box<2>({1, 1}, {2, 4}), // -0
                                                        Box<2>({3, 1}, {4, 4}), // +0
                                                        Box<2>({2, 1}, {3, 2}), // -1
                                                        Box<2>({2, 3}, {3, 4})) // +1
    );

    constexpr Box<2> box_c({1, 3}, {10, 15});
    constexpr Box<2> box_d({2, 7}, {7, 12});
    EXPECT_THAT(box_c.diff(box_d), UnorderedElementsAre(Box<2>({7, 3}, {10, 15}), // -0
                                                        Box<2>({1, 3}, {2, 15}),  // +0
                                                        Box<2>({2, 12}, {7, 15}), // -1
                                                        Box<2>({2, 3}, {7, 7}))); // +1
}

TEST(TestBox, diff2) {
    constexpr Box<2> box{{817, 846}, {1134, 1105}};
    const List<Box<2>> rem{{{1100, 1005}, {1140, 1045}},
                           {{1063, 1005}, {1103, 1045}},
                           {{1024, 1005}, {1064, 1045}},
                           {{987, 1006}, {1027, 1046}}};
    List<Box<2>> diff = box.diff(rem);
    for (auto &b : diff) {
        std::cout << "Box<2>" << b << "," << std::endl;
    }
}

TEST(TestBox, to_string) {
    constexpr Box<2> a({2, 3}, {7, 8});
    EXPECT_EQ(a.to_string(), "{{2, 3}, {7, 8}}");
}

/// Generic fuzz testing across N dimensional diffing
template <U64 N>
struct FuzzBoxDiff : nvl::test::FuzzingTestFixture<List<Box<N>>, Box<N>, Box<N>> {
    FuzzBoxDiff() {
        using nvl::Distribution;
        using nvl::Random;

        this->num_tests = 1E5;
        this->in[0] = Distribution::Uniform<I64>(1, 15);
        this->in[1] = Distribution::Uniform<I64>(1, 15);

        this->fuzz([](List<Box<N>> &diff, const Box<N> &a, const Box<N> &b) { diff = a.diff(b); });

        this->verify([&](const List<Box<N>> &diff, const Box<N> &a, const Box<N> &b) {
            // Confirm that we get no more than 2*N boxes
            ASSERT(diff.size() <= 2 * N, "[DIFF] a: " << a << " b: " << b << "\n"
                                                      << "  Remainders: " << diff << "\n"
                                                      << "  Resulted in more remainders than expected.");

            // Confirm that all points in `a` are in the remainder boxes unless they are also in b
            const auto remainders = nvl::Range(diff.begin(), diff.end());
            for (const Pos<N> &pt : a) {
                if (b.contains(pt)) {
                    for (const auto &d : diff) {
                        ASSERT(!d.contains(pt), "[DIFF] a: " << a << " b: " << b << "\n"
                                                             << "  Remainders: " << diff << "\n"
                                                             << "  Remainder " << b << " contains " << pt
                                                             << " also in b");
                    }
                } else {
                    ASSERT(remainders.exists([&](const Box<N> &rem) { return rem.contains(pt); }),
                           "[DIFF] a: " << a << " b: " << b << "\n"
                                        << "  Remainders: " << diff << "\n"
                                        << "  No remainder contained point " << pt << " in a but not in b")
                }
            }

            // Confirm that there's no overlap between resulting diff boxes
            for (U64 i = 0; i < diff.size(); ++i) {
                const Box<N> &first = diff[i];
                for (U64 j = i + 1; j < diff.size(); ++j) {
                    const Box<N> &second = diff[j];
                    for (const Pos<N> &pt : first) {
                        ASSERT(!second.contains(pt), "[DIFF] a: " << a << " b: " << b << "\n"
                                                                  << "  Remainders: " << diff << "\n"
                                                                  << "  Remainders " << first << " and " << second
                                                                  << " had overlapping point " << pt);
                    }
                }
            }
        });
    }
};

using FuzzBoxDiff2 = FuzzBoxDiff<2>;
TEST_F(FuzzBoxDiff2, diff2) {}

using FuzzBoxDiff3 = FuzzBoxDiff<3>;
TEST_F(FuzzBoxDiff3, diff3) {}

// The verification for this is slow lol
// using FuzzBoxDiff4 = FuzzBoxDiff<4>;
// TEST_F(FuzzBoxDiff4, DISABLED_diff4) {}

} // namespace
