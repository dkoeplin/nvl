#include "a2/action/Strafe.h"

#include "a2/entity/Player.h"
#include "nvl/math/Trig.h"

namespace a2 {

Status Strafe::act(Player &player) const {
    const auto *view = player.world()->view().dyn_cast<View3D>();
    const I64 ax = std::round(dir * 2 * std::cos((view->angle - 90) * kDeg2Rad));
    const I64 az = std::round(dir * 2 * std::sin((view->angle - 90) * kDeg2Rad));
    player.v()[0] = std::clamp<I64>(player.v()[0] + ax, -Player::kMaxVelocity, Player::kMaxVelocity);
    player.v()[2] = std::clamp<I64>(player.v()[2] + az, -Player::kMaxVelocity, Player::kMaxVelocity);
    return Status::kMove;
}

} // namespace a2
