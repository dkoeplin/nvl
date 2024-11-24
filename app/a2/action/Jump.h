#pragma once

#include "a2/action/PlayerAction.h"

namespace a2 {

struct Jump final : PlayerAction {
    class_tag(Jump, PlayerAction);
    using PlayerAction::PlayerAction;
    Status act(Player &player) const override;
};

} // namespace a2
