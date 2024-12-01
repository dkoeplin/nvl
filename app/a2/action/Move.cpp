#include "a2/action/Move.h"

#include "a2/entity/Player.h"
#include "nvl/math/Trig.h"

namespace a2 {

Status Move::act(Player &player) const {
    const auto *view = player.world()->view().dyn_cast<View3D>();
    Vec<3> v = player.v().to_vec();
    v[0] += dir * 2 * std::cos(view->angle * kDeg2Rad);
    v[2] += dir * 2 * std::sin(view->angle * kDeg2Rad);
    if (v.magnitude() > Player::kMaxVelocity) {
        v *= Player::kMaxVelocity / v.magnitude();
    }
    player.v()[0] = std::round(v[0]);
    player.v()[2] = std::round(v[2]);
    return Status::kMove;
}

} // namespace a2
