#include <gtest/gtest.h>
#include <nvl/material/TestMaterial.h>

#include "nvl/entity/Block.h"
#include "nvl/test/Fuzzing.h"
#include "nvl/test/TensorWindow.h"
#include "nvl/world/World.h"

template <U64 N>
struct nvl::RandomGen<nvl::Box<N>> {
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

namespace {

using nvl::Block;
using nvl::Box;
using nvl::Color;
using nvl::Material;
using nvl::Pos;
using nvl::Tensor;
using nvl::World;
using nvl::test::TensorWindow;

TEST(TestWindow, draw) {
    TensorWindow window("Test", {10, 10});
    auto *world = window.open<World<2>>();
    world->set_hud(false);
    auto material = Material::get<nvl::TestMaterial>(Color::kBlack);
    material->outline = false;
    constexpr Box<2> box({2, 2}, {7, 7});
    world->spawn<Block<2>>(Pos<2>::zero, box, material);
    window.draw();

    Tensor<2, Color> expected({10, 10}, Color::kWhite);
    for (const Pos<2> i : box.pos_iter()) {
        expected[i] = Color::kBlack;
    }
    EXPECT_TRUE(nvl::compare_tensors(std::cout, window.tensor(), expected));
}

struct FuzzDraw : nvl::test::FuzzingTestFixture<Tensor<2, Color>, Pos<2>, Pos<2>, Box<2>> {
    FuzzDraw() = default;
};

TEST_F(FuzzDraw, draw2d) {
    using nvl::Distribution;
    using nvl::Random;

    this->num_tests = 1E4;
    this->in[0] = Distribution::Uniform<I64>(-15, 15);
    this->in[1] = Distribution::Uniform<I64>(-15, 15);
    this->in[2] = Distribution::Uniform<I64>(-15, 15);

    auto material = Material::get<nvl::TestMaterial>(Color::kBlack);
    material->outline = false;

    fuzz([material](Tensor<2, Color> &tensor, const Pos<2> &offset, const Pos<2> &loc, const Box<2> &box) {
        TensorWindow window("Test", {10, 10});
        auto *world = window.open<World<2>>();
        world->set_hud(false);
        world->set_view(offset);
        world->spawn<Block<2>>(loc, box, material);
        window.draw();
        tensor = window.tensor();
    });

    verify([&](const Tensor<2, Color> &tensor, const Pos<2> &offset, const Pos<2> &loc, const Box<2> &box) {
        Tensor<2, Color> expected({10, 10}, Color::kWhite);
        const Box<2> expected_box = box + loc - offset;
        for (const Pos<2> i : expected_box.pos_iter()) {
            if (expected.has(i)) {
                expected[i] = Color::kBlack;
            }
        }
        if (expected != tensor) {
            EXPECT_TRUE(nvl::compare_tensors(std::cout, tensor, expected))
                << "Mismatched for offset=" << offset << ", loc=" << loc << ", box=" << box << std::endl
                << "(box + loc) - offset = " << expected_box;
            std::cout << "Expected:" << std::endl;
            nvl::test::print_10x10_tensor(expected);
            std::cout << "Actual: " << std::endl;
            nvl::test::print_10x10_tensor(tensor);
        }
    });
}

} // namespace
