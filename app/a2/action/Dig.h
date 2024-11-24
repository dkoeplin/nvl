#pragma once

#include "a2/action/PlayerAction.h"

namespace a2 {

struct Dig final : PlayerAction {
    class_tag(Dig, PlayerAction);
    using PlayerAction::PlayerAction;
    Status act(Player &player) const override;
};

} // namespace a2
