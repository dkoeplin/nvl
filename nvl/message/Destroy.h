#pragma once

#include "nvl/message/Message.h"

namespace nvl {

/**
 * @struct Destroy
 * @brief Notification to destroy the receiver(s).
 */
struct Destroy : AbstractMessage {
    class_tag(Destroy);
};

} // namespace nvl
