#pragma once

#include "nvl/message/Message.h"

namespace nvl {

/**
 * @struct Destroy
 * @brief Notification to destroy the receiver(s).
 */
struct Destroy final : AbstractMessage {
    class_tag(Destroy, AbstractMessage);
    explicit Destroy(const Actor &src) : AbstractMessage(src) {}
};

} // namespace nvl
