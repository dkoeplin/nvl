#include "nvl/ui/ViewOffset.h"

#include "nvl/math/Trig.h"

namespace nvl {

void View3D::rotate(const Pos<2> &delta, const Pos<2> &shape) {
    const float scale_x = 180 / static_cast<float>(shape[0]); // deg per pixel
    const float scale_y = 180 / static_cast<float>(shape[1]); // deg per pixel
    const float delta_x = -scale_x * delta[0];
    const float delta_y = scale_y * delta[1];
    angle = std::remainder(angle + delta_x, 360.0f);
    pitch = std::clamp(pitch + delta_y, -89.0f, 89.0f);
}

Pos<3> View3D::project() const { return project(offset, dist); }

Pos<3> View3D::project(const U64 length) const { return project(offset, length); }

Pos<3> View3D::project(const Pos<3> &from, const U64 length) const {
    // TODO: Check math when pitch is +/-90 - seeing weird behavior in that case
    // The pitch forms a cone, where the angle of the pitch defines the circle where the xz angle can rotate on.
    const float len = static_cast<float>(length);
    const float angle_rad = angle * kDeg2Rad;
    const float pitch_rad = pitch * kDeg2Rad;
    const float xz_len = len * std::cos(pitch_rad);
    const Pos<3> delta{static_cast<I64>(std::round(xz_len * std::cos(angle_rad))),
                       static_cast<I64>(std::round(len * std::sin(pitch_rad))),
                       static_cast<I64>(std::round(xz_len * std::sin(angle_rad)))};
    return from + delta;
}

} // namespace nvl
