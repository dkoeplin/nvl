#include "a2/ui/PlayerControls.h"

#include "a2/action/Brake.h"
#include "a2/action/Dig.h"
#include "a2/action/Jump.h"
#include "a2/action/Move.h"
#include "a2/action/Strafe.h"
#include "a2/entity/Player.h"
#include "nvl/actor/Actor.h"

namespace a2 {

void PlayerControls::tick() {
    Actor player(world_->player);
    if (window_->pressed(Key::Space)) {
        world_->send<Jump>(nullptr, player);
    }
    if (window_->down(Mouse::Left)) {
        world_->send<Dig>(nullptr, player);
    }
    bool no_strafe = false, no_move = false;
    if (window_->pressed(Key::A)) {
        world_->send<Strafe>(nullptr, player, Dir::Neg);
    } else if (window_->pressed(Key::D)) {
        world_->send<Strafe>(nullptr, player, Dir::Pos);
    } else {
        no_strafe = true;
    }
    if (window_->pressed(Key::W)) {
        world_->send<Move>(nullptr, player, Dir::Pos);
    } else if (window_->pressed(Key::S)) {
        world_->send<Move>(nullptr, player, Dir::Neg);
    } else {
        no_move = true;
    }
    if (no_strafe && no_move) {
        world_->send<Brake>(nullptr, player);
    }
}

void PlayerControls::draw() {}

} // namespace a2
