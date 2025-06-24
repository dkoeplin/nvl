#pragma once

#include "a2/action/PlayerAction.h"

namespace a2 {

struct Brake final : PlayerAction {
    class_tag(Brake, PlayerAction);
    using PlayerAction::PlayerAction;
    Status act(Player &player) const override;
    pure std::string to_string() const override { return "Brake"; }
};
} // namespace a2
