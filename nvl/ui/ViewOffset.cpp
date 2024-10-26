#include "nvl/ui/ViewOffset.h"

#include "nvl/math/Trig.h"

namespace nvl {

void View3D::rotate(const Pos<2> &delta, const Pos<2> &shape) {
    const float scale_x = 180 / static_cast<float>(shape[0]); // deg per pixel
    const float scale_y = 180 / static_cast<float>(shape[1]); // deg per pixel
    const float delta_x = -scale_x * delta[0];
    const float delta_y = scale_y * delta[1];
    angle = std::remainder(angle + delta_x, 360);
    pitch = std::clamp<float>(pitch + delta_y, -89, 89);
}

Pos<3> View3D::project(const U64 length) const { return project(offset, length); }

Pos<3> View3D::project(const Pos<3> &from, const U64 length) const {
    // Fun trigonometry here. The pitch forms a triangle with the XZ plane, where the angle of the pitch
    // defines the circle where the xz angle can rotate on.
    const float len = static_cast<float>(length);
    const float angle_rad = angle * kDeg2Rad;
    const float pitch_rad = pitch * kDeg2Rad;
    const float xz_len = len * std::cos(pitch_rad);

    return {static_cast<I64>(std::round(static_cast<float>(from[0]) + xz_len * std::cos(angle_rad))),
            static_cast<I64>(std::round(static_cast<float>(from[1]) + len * std::sin(pitch_rad))),
            static_cast<I64>(std::round(static_cast<float>(from[2]) + xz_len * std::sin(angle_rad)))};
}

} // namespace nvl
