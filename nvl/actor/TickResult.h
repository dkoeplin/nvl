#pragma once

#include "nvl/actor/Status.h"
#include "nvl/data/List.h"
#include "nvl/message/Message.h"

namespace nvl {

class TickResult {
public:
    Status status;
    List<Message> messages;
};

} // namespace nvl
