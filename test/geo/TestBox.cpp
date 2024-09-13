#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "nvl/geo/Box.h"
#include "nvl/geo/Pos.h"

namespace {

using testing::ElementsAre;

using nvl::Box;
using nvl::List;
using nvl::Pos;

TEST(TestBox, shape) {
    const Box<2> a({4, 5}, {32, 45});
    EXPECT_THAT(a.shape(), ElementsAre(29, 41));

    const Box<2> b({2, 4}, {6, 10});
    EXPECT_THAT(b.shape(), ElementsAre(5, 7));

    const Box<3> c({5, 8, 3}, {10, 13, 9});
    EXPECT_THAT(c.shape(), ElementsAre(6, 6, 7));
}

TEST(TestBox, mul) {
    const Box<2> a{{2, 3}, {5, 6}};
    constexpr Pos<2> b{5, -1};
    EXPECT_EQ(a * b, Box<2>({10, -6}, {25, -3}));
    EXPECT_EQ(a * 2, Box<2>({4, 6}, {10, 12}));
}

TEST(TestBox, add) {
    const Box<2> a({2, 3}, {7, 8});
    constexpr Pos<2> b{4, 2};
    EXPECT_EQ(a + b, Box<2>({6, 5}, {11, 10}));
    EXPECT_EQ(a + 5, Box<2>({7, 8}, {12, 13}));
}

TEST(TestBox, sub) {
    const Box<2> a({2, 3}, {7, 8});
    constexpr Pos<2> b{4, 2};
    EXPECT_EQ(a - b, Box<2>({-2, 1}, {3, 6}));
    EXPECT_EQ(a - 4, Box<2>({-2, -1}, {3, 4}));
}

TEST(TestBox, pos_iter) {
    constexpr auto a = Box<2>({2, 4}, {4, 8});
    const auto iter0 = a.pos_iter();
    const List<Pos<2>> list0(iter0.begin(), iter0.end());
    const List<Pos<2>> expected0{{2, 4}, {2, 5}, {2, 6}, {2, 7}, {2, 8}, {3, 4}, {3, 5}, {3, 6},
                                 {3, 7}, {3, 8}, {4, 4}, {4, 5}, {4, 6}, {4, 7}, {4, 8}};
    EXPECT_EQ(list0, expected0);

    const auto iter1 = a.pos_iter(/*step*/ 2);
    const List<Pos<2>> list1(iter1.begin(), iter1.end());
    const List<Pos<2>> expected1{{2, 4}, {2, 6}, {2, 8}, {4, 4}, {4, 6}, {4, 8}};
    EXPECT_EQ(list1, expected1);

    const auto iter2 = a.pos_iter({1, 2});
    const List<Pos<2>> list2(iter2.begin(), iter2.end());
    const List<Pos<2>> expected2{{2, 4}, {2, 6}, {2, 8}, {3, 4}, {3, 6}, {3, 8}, {4, 4}, {4, 6}, {4, 8}};
    EXPECT_EQ(list2, expected2);

    EXPECT_DEATH({ std::cout << a.pos_iter({0, 2}); }, "Invalid iterator step size of 0");
    EXPECT_DEATH({ std::cout << a.pos_iter({-1, 2}); }, "TODO: Support negative step");
}

TEST(TestBox, box_iter) {
    const Box<2> a({2, 2}, {6, 8}); // shape is 5x7

    const auto iter0 = a.box_iter({2, 2});
    const List<Box<2>> list0(iter0.begin(), iter0.end());
    const List<Box<2>> expected0{
        Box<2>({2, 2}, {3, 3}), // row 0:1, col 0:1
        Box<2>({2, 4}, {3, 5}), // row 0:1, col 2:3
        Box<2>({2, 6}, {3, 7}), // row 0:1, col 4:5
        Box<2>({4, 2}, {5, 3}), // row 2:3, col 0:1
        Box<2>({4, 4}, {5, 5}), // row 2:3, col 0:1
        Box<2>({4, 6}, {5, 7})  // row 2:3, col 0:1
    };
    EXPECT_EQ(list0, expected0);

    const auto iter1 = a.box_iter({1, 3});
    const List<Box<2>> list1(iter1.begin(), iter1.end());
    const List<Box<2>> expected1{
        Box<2>({2, 2}, {2, 4}), // row 0, col 0:2
        Box<2>({2, 5}, {2, 7}), // row 0, col 3:5
        Box<2>({3, 2}, {3, 4}), // row 1, col 0:2
        Box<2>({3, 5}, {3, 7}), // row 1, col 3:5
        Box<2>({4, 2}, {4, 4}), // row 2, col 0:2
        Box<2>({4, 5}, {4, 7}), // row 2, col 3:5
        Box<2>({5, 2}, {5, 4}), // row 3, col 0:2
        Box<2>({5, 5}, {5, 7}), // row 3, col 3:5
        Box<2>({6, 2}, {6, 4}), // row 2, col 0:2
        Box<2>({6, 5}, {6, 7}), // row 2, col 3:5
    };

    EXPECT_DEATH({ std::cout << a.box_iter({0, 2}); }, "Invalid iterator shape size of 0");
    EXPECT_DEATH({ std::cout << a.box_iter({-1, 2}); }, "TODO: Support negative step");
}

TEST(TestBox, clamp) {
    EXPECT_EQ(Box<2>({0, 0}, {511, 511}).clamp({1024, 1024}), Box<2>({0, 0}, {1023, 1023}));
    EXPECT_EQ(Box<2>({0, 0}, {1023, 1023}).clamp({1024, 1024}), Box<2>({0, 0}, {1023, 1023}));
    EXPECT_EQ(Box<2>({0, 0}, {1024, 1024}).clamp({1024, 1024}), Box<2>({0, 0}, {2047, 2047}));
    EXPECT_EQ(Box<2>({512, 512}, {1023, 1023}).clamp({1024, 1024}), Box<2>({0, 0}, {1023, 1023}));
    EXPECT_EQ(Box<2>({346, -398}, {666, -202}).clamp({1024, 1024}), Box<2>({0, -1024}, {1023, -1}));
    EXPECT_EQ(Box<2>({-100, 100}, {100, 300}).clamp({1024, 1024}), Box<2>({-1024, 0}, {1023, 1023}));
}

TEST(TestBox, to_string) {
    const Box<2> a({2, 3}, {7, 8});
    EXPECT_EQ(a.to_string(), "{2, 3}::{7, 8}");
}

} // namespace
