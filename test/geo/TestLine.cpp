#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "nvl/geo/Box.h"
#include "nvl/geo/Line.h"
#include "nvl/geo/Pos.h"
#include "nvl/geo/Vec.h"
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

using nvl::Box;
using nvl::Dir;
using nvl::Face;
using nvl::Intersect;
using nvl::Line;
using nvl::Maybe;
using nvl::Vec;

TEST(TestLine, intersect) {
    constexpr Box<3> box{{500, 950, 500}, {549, 999, 549}};
    constexpr Line<3> line{{528, 969, 410}, {528, 974, 510}};
    const auto intersect = line.intersect(box);
    ASSERT_TRUE(intersect.has_value());
    EXPECT_EQ(intersect->pt, Vec<3>(528, 973.5, 500));
}

using FuzzLineIntersect = nvl::test::FuzzingTestFixture<Maybe<Intersect<2>>, Line<2>, Box<2>>;
TEST_F(FuzzLineIntersect, fuzz2d) {
    num_tests = 1E3;
    in[0] = nvl::Distribution::Uniform<I64>(-20, 20);
    in[1] = nvl::Distribution::Uniform<I64>(-15, 15);
    fuzz([](Maybe<Intersect<2>> &x, const Line<2> &line, const Box<2> &box) { x = line.intersect(box); });

    verify([](const Maybe<Intersect<2>> &x, const Line<2> &line, const Box<2> &box) {
        /*std::cout << line << " + " << box << " = ";
        if (x.has_value()) {
            std::cout << x->pt << " (dist: " << x->dist << ", face: " << x->dir << x->dim << ")";
        } else {
            std::cout << "N/A";
        }
        std::cout << std::endl;*/
        if (box.contains(line.a)) {
            ASSERT(x->pt == line.a.to_vec(), "Expected A to be the intersection when A is inside the box.");
            ASSERT(x->dist == 0, "Expected a distance of 0 when A is inside the box.");
            ASSERT(x->face == Face(Dir::Pos, 0), "Expected face +0 when A is inside the box.");
        }

        // Confirm using y = mx + b, m = (y1 - y0)/(x1 - x0), b = y0 - m*x0
        // This is pretty much just a simplified version of what the intersect code does
        // F64 m = static_cast<F64>(line.b[1] - line.a[1]) / (line.b[0] - line.a[0]);
        // F64 b = line.a[1] - m * line.a[0];
    });
}

} // namespace
