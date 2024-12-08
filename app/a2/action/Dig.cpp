#include "a2/action/Dig.h"

#include "a2/entity/Player.h"

namespace a2 {

Status Dig::act(Player &player) const {
    const World<3> *world = player.world();
    const U64 now = world->ticks();
    if (now >= player.last_dig + player.dig_ticks()) {
        const auto *view = world->view().dyn_cast<View3D>();
        const Vec<3> &start = real(view->offset);
        const Vec<3> end = view->project(player.dig_reach());
        const Line sight(start, end);
        if (const auto itx = world->first_except(sight, player.self())) {
            const Pos<3> pt = round(itx->pt);
            const Box<3> dig_box(pt - 5, pt + 5);
            player.send<Hit<3>>(itx->actor, dig_box, 1);
            player.last_dig = now;
        }
    }
    return Status::kNone;
}

} // namespace a2
