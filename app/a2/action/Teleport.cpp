#include "a2/action/Teleport.h"

#include "a2/entity/Player.h"

namespace a2 {

Status Teleport::act(Player &player) const {
    player.x() = dst;
    return Status::kMove;
}

} // namespace a2
