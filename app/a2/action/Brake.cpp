#include "a2/action/Brake.h"

#include "a2/entity/Player.h"

namespace a2 {

Status Brake::act(Player &player) const {

    if (player.has_below()) {
        Vec<3> v = real(player.v());
        v[1] = 0;
        const double v0 = v.magnitude();
        const double v1 = v0 - player.walk_accel();
        v[0] *= v1 / v0;
        v[1] *= v1 / v0;
    }
    player.v()[0] = v0 > 0 ? std::max<I64>(v0 - a, 0) : std::min<I64>(v0 + a, 0);
    player.v()[2] = v2 > 0 ? std::max<I64>(v2 - a, 0) : std::min<I64>(v2 + a, 0);
    return Status::kMove;
}

} // namespace a2
