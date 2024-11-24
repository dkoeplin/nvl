#pragma once

#include "a2/action/PlayerAction.h"

namespace a2 {

struct Brake final : PlayerAction {
    class_tag(Brake, PlayerAction);
    using PlayerAction::PlayerAction;
    Status act(Player &player) const override;
};
} // namespace a2
