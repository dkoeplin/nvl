#include "nvl/ui/ViewOffset.h"

#include "nvl/math/Trig.h"

namespace nvl {

void View3D::rotate(const Pos<2> &delta, const Pos<2> &shape) {
    const F64 scale_x = 180.0 / static_cast<F64>(shape[0]); // deg per pixel
    const F64 scale_y = 180.0 / static_cast<F64>(shape[1]); // deg per pixel
    const F64 delta_x = -scale_x * static_cast<F64>(delta[0]);
    const F64 delta_y = scale_y * static_cast<F64>(delta[1]);
    angle = std::remainder(angle + delta_x, 360.0);
    pitch = std::clamp(pitch + delta_y, -89.0, 89.0);
}

Vec<3> View3D::project() const { return project(real(offset), dist); }

Vec<3> View3D::project(const F64 length) const { return project(real(offset), length); }

Vec<3> View3D::project(const Vec<3> &from, const F64 length) const {
    // TODO: Check math when pitch is +/-90 - seeing weird behavior in that case
    // The pitch forms a cone, where the angle of the pitch defines the circle where the xz angle can rotate on.
    const F64 angle_rad = angle * kDeg2Rad;
    const F64 pitch_rad = pitch * kDeg2Rad;
    const F64 xz_len = length * std::cos(pitch_rad);
    const Vec<3> delta{xz_len * std::cos(angle_rad), length * std::sin(pitch_rad), xz_len * std::sin(angle_rad)};
    return from + delta;
}

} // namespace nvl
