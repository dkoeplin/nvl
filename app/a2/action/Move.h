#pragma once

#include "a2/action/PlayerAction.h"
#include "nvl/actor/Actor.h"
#include "nvl/geo/Dir.h"

namespace a2 {

struct Move final : PlayerAction {
    class_tag(Move, PlayerAction);
    explicit Move(AbstractActor *src, const Dir dir) : PlayerAction(src), dir(dir) {}
    Status act(Player &player) const override;
    pure std::string to_string() const override { return "Move(" + dir.to_string() + ")"; }
    Dir dir;
};

} // namespace a2
