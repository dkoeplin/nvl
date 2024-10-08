#include <gtest/gtest.h>
#include <nvl/material/TestMaterial.h>

#include "nvl/entity/Block.h"
#include "nvl/test/TensorWindow.h"
#include "nvl/world/World.h"

namespace {

using nvl::Block;
using nvl::Box;
using nvl::Color;
using nvl::Material;
using nvl::Pos;
using nvl::Tensor;
using nvl::World;
using nvl::test::TensorWindow;

void print_tensor(const Tensor<2, Color> &tensor) {
    std::cout << "  0123456789" << std::endl;
    std::cout << "------------" << std::endl;
    for (int64_t i = 0; i < 10; ++i) {
        std::cout << i << "|";
        for (int64_t j = 0; j < 10; ++j) {
            auto elem = tensor[{i, j}];
            std::cout << (elem == Color::kBlack ? "X" : ".");
        }
        std::cout << std::endl;
    }
}

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

} // namespace
