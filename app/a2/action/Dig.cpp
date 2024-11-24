#include "a2/action/Dig.h"

#include "a2/entity/Player.h"

namespace a2 {

Status Dig::act(Player &player) const {
    const World<3> *world = player.world();
    const U64 now = world->ticks();
    if (now >= player.last_dig + Player::kDigTicks) {
        const auto *view = world->view().dyn_cast<View3D>();
        const Pos<3> &start = view->offset;
        const Pos<3> end = view->project(Player::kDigDist);
        const Line sight(start, end);
        if (const auto itx = world->first_except(sight, player.self())) {
            const Pos<3> pt = Pos<3>::round(itx->pt);
            const Box<3> dig_box(pt - 5, pt + 5);
            player.send<Hit<3>>(itx->actor, dig_box, 1);
            player.last_dig = now;
        }
    }
    return Status::kNone;
}

} // namespace a2
