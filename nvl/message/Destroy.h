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

    explicit Destroy(AbstractActor *src, const Cause cause) : AbstractMessage(src), cause(cause) {}

    pure std::string to_string() const override { return "Destroy"; }

    Cause cause;
};

} // namespace nvl
