#pragma once

#include "nvl/message/Message.h"

namespace nvl {

/**
 * @struct Created
 * @brief Notification that the recipient was created by the source.
 */
struct Created final : AbstractMessage {
    class_tag(Created, AbstractMessage);
    using AbstractMessage::AbstractMessage;
    pure std::string to_string() const override { return "Created"; }
};

} // namespace nvl
