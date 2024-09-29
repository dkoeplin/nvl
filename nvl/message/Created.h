#pragma once

#include "nvl/message/Message.h"

namespace nvl {

/**
 * @struct Created
 * @brief Notification that the recipient was created by the source.
 */
struct Created final : AbstractMessage {
    class_tag(Created, AbstractMessage);
    explicit Created(const Actor &src) : AbstractMessage(src) {}
};

} // namespace nvl
