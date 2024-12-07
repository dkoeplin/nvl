#include "a2/action/Jump.h"

#include "a2/entity/Player.h"
#include "a2/macros/Literals.h"

namespace a2 {

Status Jump::act(Player &player) const {
    if (player.has_below() && player.v()[1] == 0) {
        player.v()[1] = -30_mps;
        return Status::kMove;
    }
    return Status::kNone;
}

} // namespace a2
