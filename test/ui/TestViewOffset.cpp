#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "nvl/math/Trig.h"
#include "nvl/ui/ViewOffset.h"

namespace {

using nvl::PI;
using nvl::Pos;
using nvl::View3D;
using testing::ElementsAre;

TEST(TestViewOffset, view3d_rotate) {
    View3D view;
    view.offset = Pos<3>::zero;
    view.pitch = 45;
    view.angle = 45;
    // Move the "mouse" by 10 pixels to the right
    view.rotate({10, 0}, {1000, 1000});
    EXPECT_NEAR(view.pitch, 45.0f, 0.1f);
    EXPECT_NEAR(view.angle, 43.2f, 0.1f);

    // Move the "mouse" by 10 pixels down
    view.rotate({0, 100}, {1000, 1000});
    EXPECT_NEAR(view.pitch, 63.0f, 0.1f);
    EXPECT_NEAR(view.angle, 43.2f, 0.1f);

    // To the right by 1000 pixels
    view.rotate({1000, 0}, {1000, 1000});
    EXPECT_NEAR(view.angle, -136.8f, 0.1f);
    view.rotate({-1000, 0}, {1000, 1000});
    EXPECT_NEAR(view.angle, 43.2f, 0.1f);
}

TEST(TestViewOffset, view3d_project) {
    View3D view;
    view.offset = Pos<3>::zero;
    view.pitch = 45;
    view.angle = 45;
    const float x = std::round(100 * cos(PI / 4) * cos(PI / 4));
    const float y = std::round(100 * sin(PI / 4));
    const float z = std::round(100 * cos(PI / 4) * sin(PI / 4));
    EXPECT_THAT(view.project(100), ElementsAre(x, y, z));
}

} // namespace
