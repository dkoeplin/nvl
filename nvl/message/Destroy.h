#pragma once

#include "nvl/message/Message.h"

namespace nvl {

/**
 * @struct Destroy
 * @brief Notification to destroy the receiver(s).
 */
struct Destroy final : AbstractMessage {
    class_tag(Destroy, AbstractMessage);

    enum Cause { kOutOfBounds, kRemoved };

    explicit Destroy(AbstractActor *src, Cause cause) : AbstractMessage(src), cause(cause) {}

    Cause cause;
};

} // namespace nvl
