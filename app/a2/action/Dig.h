#pragma once

#include "a2/action/PlayerAction.h"

namespace a2 {

struct Dig final : PlayerAction {
    class_tag(Dig, PlayerAction);
    using PlayerAction::PlayerAction;
    Status act(Player &player) const override;
    pure std::string to_string() const override { return "Dig"; }
};

} // namespace a2
