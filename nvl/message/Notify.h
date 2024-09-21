#pragma once

#include "nvl/message/Message.h"

namespace nvl {

/**
 * @struct Notify
 * @brief General wake up / change notification.
 */
struct Notify : AbstractMessage {
    class_tag(Notify);
    enum Cause { kRemoved, kBroken, kMoved, kOther };
    Cause cause;
};

} // namespace nvl
