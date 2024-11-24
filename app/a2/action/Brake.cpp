#include "a2/action/Brake.h"

#include "a2/entity/Player.h"

namespace a2 {

Status Brake::act(Player &player) const {
    const I64 v0 = player.v()[0];
    const I64 v2 = player.v()[2];
    player.v()[0] = v0 > 0 ? std::max<I64>(v0 - 2, 0) : std::min<I64>(v0 + 2, 0);
    player.v()[2] = v2 > 0 ? std::max<I64>(v2 - 2, 0) : std::min<I64>(v2 + 2, 0);
    return Status::kMove;
}

} // namespace a2
