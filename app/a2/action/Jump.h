#pragma once

#include "a2/action/PlayerAction.h"

namespace a2 {

struct Jump final : PlayerAction {
    class_tag(Jump, PlayerAction);
    using PlayerAction::PlayerAction;
    Status act(Player &player) const override;
    pure std::string to_string() const override { return "Jump"; }
};

} // namespace a2
