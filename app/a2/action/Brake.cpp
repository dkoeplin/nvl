#include "a2/action/Brake.h"

#include "a2/entity/Player.h"

namespace a2 {

Status Brake::act(Player &player) const {
    if (player.has_below()) {
        const F64 a = static_cast<F64>(player.walk_accel());
        const Vec<3> v = real(player.v()).with(World<3>::kVerticalDim, 0);
        const F64 speed = v.magnitude();
        const F64 speed2 = speed > 0 ? std::max(0.0, speed - a) : std::min(0.0, speed + a);
        const Line<3> line_v(Vec<3>::zero, v);
        const Pos<3> v2 = round(line_v.interpolate(speed2));
        player.v()[0] = v2[0];
        player.v()[2] = v2[2];
    }
    return Status::kMove;
}

} // namespace a2
