#include <gtest/gtest.h>

#include "nvl/geo/RBox.h"
#include "nvl/math/Trig.h"

namespace {

using nvl::Pos;
using nvl::RBox;
using nvl::Rotation;

TEST(TestRBox, points2d) {
    RBox box({{0, 0}, {5, 5}}, Rotation<2>::zero);
    std::cout << box.points() << std::endl;

    box.rotate({45});
    std::cout << box << std::endl;
    std::cout << box.points() << std::endl;

    box.rotate({-45});
    std::cout << box.points() << std::endl;
}

} // namespace
