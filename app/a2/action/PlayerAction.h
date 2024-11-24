#pragma once

#include "nvl/actor/Status.h"
#include "nvl/message/Message.h"
#include "nvl/reflect/ClassTag.h"

namespace a2 {
using namespace nvl;

struct Player;

struct PlayerAction : AbstractMessage {
    class_tag(PlayerAction, AbstractMessage);
    using AbstractMessage::AbstractMessage;
    virtual Status act(Player &player) const = 0;
};

} // namespace a2
