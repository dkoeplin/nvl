#include "a2/action/Strafe.h"

#include "a2/entity/Player.h"
#include "nvl/math/Trig.h"

namespace a2 {

Status Strafe::act(Player &player) const {
    const auto *view = player.world()->view().dyn_cast<View3D>();
    const auto max = player.walk_max_velocity();
    const auto a = player.walk_accel();
    Vec<3> v = real(player.v());
    v[0] += dir * a * std::cos((view->angle - 90) * kDeg2Rad);
    v[2] += dir * a * std::sin((view->angle - 90) * kDeg2Rad);
    if (v.magnitude() > max) {
        v *= max / v.magnitude();
    }
    player.v()[0] = static_cast<I64>(std::round(v[0]));
    player.v()[2] = static_cast<I64>(std::round(v[2]));
    return Status::kMove;
}

} // namespace a2
