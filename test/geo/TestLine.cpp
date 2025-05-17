#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "nvl/geo/Line.h"
#include "nvl/geo/Tuple.h"
#include "nvl/geo/Volume.h"
#include "nvl/math/Distribution.h"
#include "nvl/math/Random.h"
#include "nvl/test/Fuzzing.h"

namespace nvl {

template <U64 N>
struct RandomGen<Box<N>> {
    template <typename I>
    pure Box<N> uniform(Random &random, const I min, const I max) const {
        const auto a = random.uniform<Pos<N>, I>(min, max);
        const auto b = random.uniform<Pos<N>, I>(min, max);
        return Box<N>(a, b);
    }
    template <typename I>
    pure Box<N> normal(Random &random, const I mean, const I stddev) const {
        const auto a = random.normal<Pos<N>, I>(mean, stddev);
        const auto b = random.normal<Pos<N>, I>(mean, stddev);
        return Box<N>(a, b);
    }
};

template <U64 N>
struct RandomGen<Line<N>> {
    pure Line<N> uniform(Random &random, const F64 min, const F64 max) const {
        const auto a = random.uniform<Vec<N>, F64>(min, max);
        const auto b = random.uniform<Vec<N>, F64>(min, max);
        return Line<N>(a, b);
    }
    pure Line<N> normal(Random &random, const F64 mean, const F64 stddev) const {
        const auto a = random.normal<Vec<N>, F64>(mean, stddev);
        const auto b = random.normal<Vec<N>, F64>(mean, stddev);
        return Line<N>(a, b);
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

#define EXPECT_VEC_NEAR(actual, expected, error)                                                                       \
    for (U64 i = 0; i < actual.rank(); ++i) {                                                                          \
        EXPECT_NEAR(actual[i], expected[i], error);                                                                    \
    }

TEST(TestLine, length) {
    const Line<2> a(Vec<2>(1, 3), Vec<2>(3, 6));
    const F64 expected_len = std::sqrt(2 * 2 + 3 * 3);
    EXPECT_NEAR(a.length(), expected_len, 0.01 * expected_len);
}

TEST(TestLine, slope) {
    const Line<2> a(Vec<2>(1, 3), Vec<2>(3, 6));
    const F64 expected_len = std::sqrt(2 * 2 + 3 * 3);
    const Vec<2> expected_slope = Vec<2>(2, 3) / expected_len;
    EXPECT_VEC_NEAR(a.slope(), expected_slope, 0.001);
}

TEST(TestLine, intersect) {
    constexpr Box<3> box{{500, 950, 500}, {550, 1000, 550}};
    const Line<3> line{Vec<3>{528, 969, 410}, Vec<3>{528, 974, 510}};
    const auto intersect = line.intersect(box);
    ASSERT_TRUE(intersect.has_value());
    EXPECT_EQ(intersect->pt, Vec<3>(528, 973.5, 500));
}

TEST(TestLine, interpolate) {
    const Line<2> line{{1, 1}, {4, 5}};
    EXPECT_EQ(line.interpolate(-5), Vec<2>(-2, -3)); // Not on the line segment
    EXPECT_EQ(line.interpolate(0), Vec<2>(1, 1));
    EXPECT_EQ(line.interpolate(5), Vec<2>(4, 5));
    EXPECT_EQ(line.interpolate(10), Vec<2>(7, 9)); // Not on the line segment
}

TEST(TestLine, profile_intersect) {
    constexpr U64 kNumTests = 1E6;
    nvl::Random random(0xFEEDBEEF);
    const auto lines_gen = nvl::Distribution::Uniform<F64>(-20, 20);
    const auto boxes_gen = nvl::Distribution::Uniform<I64>(-15, 15);

    nvl::List<Line<3>> lines;
    nvl::List<Box<3>> boxes;
    lines.reserve(kNumTests);
    boxes.reserve(kNumTests);
    for (U64 i = 0; i < kNumTests; ++i) {
        lines.push_back(lines_gen.next<Line<3>>(random));
        boxes.push_back(boxes_gen.next<Box<3>>(random));
    }

    nvl::List<Maybe<Intersect<3>>> results;
    results.reserve(kNumTests);
    const auto start = nvl::Clock::now();
    for (U64 i = 0; i < kNumTests; ++i) {
        results.push_back(lines[i].intersect(boxes[i]));
    }
    const auto stop = nvl::Clock::now();
    const auto time = nvl::Duration(stop - start);
    std::cout << "Total time:  " << time << std::endl;
    std::cout << "Time / call: " << time / kNumTests << std::endl;
}

template <U64 N>
struct FuzzLineIntersect : nvl::test::FuzzingTestFixture<Maybe<Intersect<N>>, Line<N>, Box<N>> {
    FuzzLineIntersect() {
        this->num_tests = 1E6;
        this->in[0] = nvl::Distribution::Uniform<F64>(-20, 20);
        this->in[1] = nvl::Distribution::Uniform<I64>(-15, 15);
        this->fuzz([](Maybe<Intersect<N>> &x, const Line<N> &line, const Box<N> &box) { x = line.intersect(box); });

        this->verify(
            [](const Maybe<Intersect<N>> &x, const Line<N> &line, const Box<N> &box) {
                if (box.contains(line.a())) {
                    ASSERT_TRUE(x.has_value()) //
                        << "Expected A to be the intersection when A is inside the box." << std::endl
                        << "Box: " << box << std::endl
                        << "Line: " << line;
                    EXPECT_EQ(x->pt, line.a()) //
                        << "Expected A to be the intersection when A is inside the box." << std::endl
                        << "Box: " << box << std::endl
                        << "Line: " << line;
                    EXPECT_EQ(x->dist, 0) //
                        << "Expected a distance of 0 when A is inside the box." << std::endl
                        << "Box: " << box << std::endl
                        << "Line: " << line;
                    EXPECT_FALSE(x->face.has_value()) //
                        << "Expected no face when A is inside box: " << std::endl
                        << "Box: " << box << std::endl
                        << "Line: " << line;
                } else {
                    if (x.has_value()) {
                        EXPECT_GT(x->dist, 0) //
                            << "Expected intersection distance to be > 0." << std::endl
                            << "Box: " << box << std::endl
                            << "Line: " << line << std::endl
                            << "Length: " << line.length() << std::endl
                            << "Intersect pt: " << x->pt << std::endl
                            << "Intersect dist: " << x->dist << std::endl
                            << "Intersect face: " << x->face.value() << std::endl;
                        EXPECT_LE(x->dist, line.length()) //
                            << "Expected intersection distance to be <= line length." << std::endl
                            << "Box: " << box << std::endl
                            << "Line: " << line << std::endl
                            << "Length: " << line.length() << std::endl
                            << "Intersect pt: " << x->pt << std::endl
                            << "Intersect dist: " << x->dist << std::endl
                            << "Intersect face: " << x->face.value() << std::endl;

                        const auto pt = line.interpolate(x->dist);
                        for (U64 i = 0; i < 2; ++i) {
                            EXPECT_NEAR(pt[i], x->pt[i], 0.001) //
                                << "Expected intersection point to be near interpolation point at dist." << std::endl
                                << "Box: " << box << std::endl
                                << "Line: " << line << std::endl
                                << "Length: " << line.length() << std::endl
                                << "Slope: " << line.slope() << std::endl
                                << "Intersect pt: " << x->pt << std::endl
                                << "Intersect dist: " << x->dist << std::endl
                                << "Intersect face: " << x->face.value() << std::endl
                                << "Interpolated point: " << pt << std::endl;
                        }
                        EXPECT_TRUE(box.contains(x->pt)) //
                            << "Expected intersection point to be contained within box." << std::endl
                            << "Box: " << box << std::endl
                            << "Line: " << line << std::endl
                            << "Intersect pt: " << x->pt << std::endl
                            << "Intersect dist: " << x->dist << std::endl
                            << "Intersect face: " << x->face.value() << std::endl;

                        // Expect that the intersection point is at one of the surfaces of the box
                        bool has_face = false;
                        for (U64 d = 0; d < N; ++d) {
                            has_face |= (x->pt[d] == box.min[d] || x->pt[d] == box.max_f64()[d]);
                        }
                        EXPECT_TRUE(has_face) //
                            << "Expected intersection point to be on a face of the box." << std::endl
                            << "Box: " << box << std::endl
                            << "Line: " << line << std::endl
                            << "Intersect pt: " << x->pt << std::endl
                            << "Intersect dist: " << x->dist << std::endl
                            << "Intersect face: " << x->face.value() << std::endl;
                    }
                    for (I64 d = 0; d < static_cast<I64>(std::ceil(line.length())) * 10; ++d) {
                        const F64 dist = static_cast<F64>(d) / 10.0;
                        const Vec<N> pt = line.interpolate(dist);
                        if (x.has_value() && dist < x->dist) {
                            ASSERT(!box.contains(pt), //
                                   "Expected interpolated point prior to intersection not to be in box."
                                       << std::endl
                                       << "Box: " << box << std::endl
                                       << "Line: " << line << std::endl
                                       << "Intersect pt: " << x->pt << std::endl
                                       << "Intersect dist: " << x->dist << std::endl
                                       << "Intersect face: " << x->face.value() << std::endl
                                       << "Interpolated dist: " << dist << std::endl
                                       << "Interpolated pt: " << pt << std::endl);
                        } else if (!x.has_value() && dist <= line.length()) {
                            ASSERT(!box.contains(pt), //
                                   "Expected no points on line in box when intersection failed."
                                       << std::endl
                                       << "Box: " << box << std::endl
                                       << "Line: " << line << std::endl
                                       << "Interpolated dist: " << dist << std::endl
                                       << "Interpolated pt: " << pt << std::endl);
                        }
                    }
                }
            });
    }
};

using FuzzIntersect2D = FuzzLineIntersect<2>;
using FuzzIntersect3D = FuzzLineIntersect<3>;
TEST_F(FuzzIntersect2D, fuzz2d) {}
TEST_F(FuzzIntersect3D, fuzz3d) {}

} // namespace
