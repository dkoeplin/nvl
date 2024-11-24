#pragma once

#include "a2/action/PlayerAction.h"
#include "nvl/actor/Actor.h"
#include "nvl/geo/Pos.h"

namespace a2 {

struct Teleport final : PlayerAction {
    class_tag(Teleport, PlayerAction);
    explicit Teleport(AbstractActor *src, const Pos<3> &dst) : PlayerAction(src), dst(dst) {}
    Status act(Player &player) const override;
    Pos<3> dst;
};

} // namespace a2
