#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "nvl/data/Map.h"
#include "nvl/geo/Pos.h"

namespace {

using nvl::Map;
using nvl::None;
using nvl::Pos;

using testing::ElementsAre;

TEST(TestPos, fill) {
    const auto a = Pos<3>::fill(8);
    EXPECT_EQ(a.rank(), 3);
    EXPECT_THAT(a, ElementsAre(8, 8, 8));

    const auto b = Pos<10>::fill(3);
    EXPECT_THAT(b, ElementsAre(3, 3, 3, 3, 3, 3, 3, 3, 3, 3));
}

TEST(TestPos, unit) {
    const auto a = Pos<3>::unit(1);
    EXPECT_THAT(a, ElementsAre(0, 1, 0));

    const auto b = Pos<5>::unit(4);
    EXPECT_THAT(b, ElementsAre(0, 0, 0, 0, 1));
}

TEST(TestPos, zero) {
    const auto z4 = Pos<4>::zero();
    EXPECT_THAT(z4, ElementsAre(0, 0, 0, 0));

    const auto z3 = Pos<3>::zero();
    EXPECT_THAT(z3, ElementsAre(0, 0, 0));
}

TEST(TestPos, constructor) {
    constexpr Pos<5> a{1, 2, 3, 4, 5};
    EXPECT_THAT(a, ElementsAre(1, 2, 3, 4, 5));
}

TEST(TestPos, get) {
    constexpr Pos<5> a{0, 1, 2, 3, 4};
    EXPECT_EQ(a.get(2), 2);
    EXPECT_EQ(a.get(-1), None);
    EXPECT_EQ(a.get(6), None);
}

TEST(TestPos, get_or) {
    constexpr Pos<5> a{0, 1, 2, 3, 4};
    EXPECT_EQ(a.get_or(2, 5), 2);
    EXPECT_EQ(a.get_or(-1, 32), 32);
    EXPECT_EQ(a.get_or(100, 55), 55);
}

TEST(TestPos, brackets_apply) {
    constexpr Pos<5> a{0, 1, 2, 3, 4};
    EXPECT_EQ(a[0], 0);
    EXPECT_EQ(a[3], 3);
    EXPECT_DEATH({ std::cout << a[-1]; }, "out of bounds \\[0, 5\\)");
}

TEST(TestPos, brackets_update) {
    Pos<5> a{0, 1, 2, 3, 4};
    a[0] = 32;
    EXPECT_EQ(a.get(0), 32);

    EXPECT_DEATH({ a[-1] = 32; }, "out of bounds \\[0, 5\\)");
}

TEST(TestPos, with) {
    constexpr Pos<5> a{0, 1, 2, 3, 4};
    EXPECT_THAT(a.with(3, 5), ElementsAre(0, 1, 2, 5, 4));
    EXPECT_THAT(a, ElementsAre(0, 1, 2, 3, 4));

    EXPECT_DEATH({ std::cout << a.with(-1, 32); }, "out of bounds \\[0, 5\\)");
}

TEST(TestPos, negate) {
    constexpr Pos<5> a{0, 1, 2, 3, 4};
    EXPECT_THAT(-a, ElementsAre(0, -1, -2, -3, -4));
    EXPECT_THAT(a, ElementsAre(0, 1, 2, 3, 4));
}

TEST(TestPos, mul) {
    Pos<5> a{0, 1, 2, 3, 4};
    Pos<5> b{4, 2, 2, 5, 3};
    EXPECT_THAT(a * b, ElementsAre(0, 2, 4, 15, 12));
    EXPECT_THAT(a * 5, ElementsAre(0, 5, 10, 15, 20));
    EXPECT_THAT(2 * a, ElementsAre(0, 2, 4, 6, 8));
    EXPECT_THAT(a, ElementsAre(0, 1, 2, 3, 4));
    EXPECT_THAT(b, ElementsAre(4, 2, 2, 5, 3));

    a *= b;
    EXPECT_THAT(a, ElementsAre(0, 2, 4, 15, 12));
    EXPECT_THAT(b, ElementsAre(4, 2, 2, 5, 3));
    a *= 2;
    EXPECT_THAT(a, ElementsAre(0, 4, 8, 30, 24));
}

TEST(TestPos, div) {
    Pos<5> a{0, 1, 2, 3, 4};
    Pos<5> b{4, 2, 2, 5, 3};
    EXPECT_THAT(a / b, ElementsAre(0, 0, 1, 0, 1));
    EXPECT_THAT(b / 2, ElementsAre(2, 1, 1, 2, 1));
    EXPECT_THAT(20 / b, ElementsAre(5, 10, 10, 4, 6));
    EXPECT_THAT(a, ElementsAre(0, 1, 2, 3, 4));
    EXPECT_THAT(b, ElementsAre(4, 2, 2, 5, 3));

    a /= b;
    EXPECT_THAT(a, ElementsAre(0, 0, 1, 0, 1));
    EXPECT_THAT(b, ElementsAre(4, 2, 2, 5, 3));
    b /= 2;
    EXPECT_THAT(b, ElementsAre(2, 1, 1, 2, 1));
}

TEST(TestPos, add) {
    Pos<5> a{0, 1, 2, 3, 4};
    Pos<5> b{4, 2, 2, 5, 3};
    EXPECT_THAT(a + b, ElementsAre(4, 3, 4, 8, 7));
    EXPECT_THAT(a + 5, ElementsAre(5, 6, 7, 8, 9));
    EXPECT_THAT(2 + a, ElementsAre(2, 3, 4, 5, 6));
    EXPECT_THAT(a, ElementsAre(0, 1, 2, 3, 4));
    EXPECT_THAT(b, ElementsAre(4, 2, 2, 5, 3));

    a += b;
    EXPECT_THAT(a, ElementsAre(4, 3, 4, 8, 7));
    EXPECT_THAT(b, ElementsAre(4, 2, 2, 5, 3));
    a += 10;
    EXPECT_THAT(a, ElementsAre(14, 13, 14, 18, 17));
}

TEST(TestPos, sub) {
    Pos<5> a{0, 1, 2, 3, 4};
    Pos<5> b{4, 2, 2, 5, 3};
    EXPECT_THAT(a - b, ElementsAre(-4, -1, 0, -2, 1));
    EXPECT_THAT(a - 2, ElementsAre(-2, -1, 0, 1, 2));
    EXPECT_THAT(2 - a, ElementsAre(2, 1, 0, -1, -2));
    EXPECT_THAT(a, ElementsAre(0, 1, 2, 3, 4));
    EXPECT_THAT(b, ElementsAre(4, 2, 2, 5, 3));

    a -= b;
    EXPECT_THAT(a, ElementsAre(-4, -1, 0, -2, 1));
    EXPECT_THAT(b, ElementsAre(4, 2, 2, 5, 3));
    a -= 2;
    EXPECT_THAT(a, ElementsAre(-6, -3, -2, -4, -1));
}

TEST(TestPos, grid_max) {
    EXPECT_EQ(Pos<2>(0, 10).grid_max({10, 10}), Pos<2>(9, 19));
    EXPECT_EQ(Pos<2>(0, -10).grid_max({10, 10}), Pos<2>(9, -1));
}

TEST(TestPos, grid_min) {
    EXPECT_EQ(Pos<2>(0, 9).grid_min({10, 10}), Pos<2>(0, 0));
    EXPECT_EQ(Pos<2>(0, -9).grid_min({10, 10}), Pos<2>(0, -10));
}

TEST(TestPos, comparisons) {
    constexpr Pos<5> a{0, 1, 2, 3, 4};
    constexpr Pos<5> b{4, 3, 2, 1, 0};
    constexpr Pos<5> c{0, 1, 2, 3, 4};
    constexpr Pos<5> d{4, 4, 5, 10, 5};
    EXPECT_EQ(a, a);
    EXPECT_EQ(b, b);
    EXPECT_EQ(c, c);
    EXPECT_EQ(a, c);
    EXPECT_NE(a, b);
    EXPECT_NE(b, c);

    EXPECT_FALSE(a.all_lt(b));
    EXPECT_FALSE(a.all_gt(b));
    EXPECT_TRUE(a.all_lte(c));
    EXPECT_TRUE(a.all_gte(c));
    EXPECT_TRUE(a.all_lte(d));
    EXPECT_TRUE(a.all_lt(d));
    EXPECT_TRUE(d.all_gt(a));
    EXPECT_TRUE(d.all_gte(a));
}

TEST(TestPos, manhattan_dist) {
    constexpr Pos<5> a{0, 1, 2, 3, 4};
    constexpr Pos<5> b{4, 3, 2, 1, 0};
    EXPECT_EQ(a.manhattan_dist(a), 0);
    EXPECT_EQ(b.manhattan_dist(b), 0);
    EXPECT_EQ(a.manhattan_dist(b), 4 + 2 + 0 + 2 + 4);
}

TEST(TestPos, dist) {
    constexpr Pos<5> a{0, 1, 2, 3, 4};
    constexpr Pos<5> b{4, 3, 2, 1, 0};
    EXPECT_EQ(a.dist(a), 0);
    EXPECT_EQ(b.dist(b), 0);
    EXPECT_EQ(a.dist(b), std::sqrt(16 + 4 + 0 + 4 + 16));
}

TEST(TestPos, magnitude) {
    constexpr Pos<5> a{0, 1, 2, 3, 4};
    constexpr Pos<5> b{4, 3, 2, 1, 0};
    EXPECT_EQ(a.magnitude(), std::sqrt(0 + 1 + 4 + 9 + 16));
    EXPECT_EQ(b.magnitude(), std::sqrt(16 + 9 + 4 + 1 + 0));
}

TEST(TestPos, to_string) {
    constexpr Pos<5> a{0, 1, 2, 3, 4};
    EXPECT_EQ(a.to_string(), "{0, 1, 2, 3, 4}");
}

TEST(TestPos, min) {
    constexpr Pos<5> a{0, 1, 2, 3, 4};
    constexpr Pos<5> b{4, 3, 2, 1, 0};
    EXPECT_THAT(min(a, b), ElementsAre(0, 1, 2, 1, 0));
}

TEST(TestPos, max) {
    constexpr Pos<5> a{0, 1, 2, 3, 4};
    constexpr Pos<5> b{4, 3, 2, 1, 0};
    EXPECT_THAT(max(a, b), ElementsAre(4, 3, 2, 3, 4));
}

TEST(TestPos, hash) {
    constexpr Pos<5> a{0, 1, 2, 3, 4};
    constexpr Pos<5> b{4, 3, 2, 1, 0};
    constexpr Pos<5> c{0, 1, 2, 3, 4};
    constexpr std::hash<Pos<5>> hash;

    Map<Pos<5>, I64> map;
    map[a] = 4;
    map[b] = 5;
    EXPECT_EQ(map[a], 4);
    EXPECT_EQ(map[b], 5);
    EXPECT_EQ(hash(a), hash(c));
}

} // namespace