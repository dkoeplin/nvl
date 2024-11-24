#pragma once

#include "nvl/message/Message.h"

namespace nvl {

/**
 * @struct Notify
 * @brief General wake up / change notification.
 */
struct Notify final : AbstractMessage {
    class_tag(Notify, AbstractMessage);
    enum Cause {
        kBroken,  // Notify recipient(s) that nearby source was broken
        kChanged, // Notify recipient(s) that nearby source was changed
        kCreated, // Notify recipient(s) that they were created by source
        kDied,    // Notify recipient(s) that nearby source died
        kMoved,   // Notify recipient(s) that nearby source moved
        kOther
    };
    explicit Notify(AbstractActor *src, const Cause cause) : AbstractMessage(src), cause(cause) {}

    Cause cause;
};

} // namespace nvl
