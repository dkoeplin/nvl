#include "a2/action/Move.h"

#include "a2/entity/Player.h"
#include "nvl/math/Trig.h"

namespace a2 {

Status Move::act(Player &player) const {
    const auto *view = player.world()->view().dyn_cast<View3D>();
    const I64 delta_x = std::round(dir * 2 * std::cos(view->angle * kDeg2Rad));
    const I64 delta_z = std::round(dir * 2 * std::sin(view->angle * kDeg2Rad));
    player.v()[0] = std::clamp<I64>(player.v()[0] + delta_x, -Player::kMaxVelocity, Player::kMaxVelocity);
    player.v()[2] = std::clamp<I64>(player.v()[2] + delta_z, -Player::kMaxVelocity, Player::kMaxVelocity);
    return Status::kMove;
}

} // namespace a2
