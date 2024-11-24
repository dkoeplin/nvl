#include "nvl/message/Message.h"

#include "nvl/actor/Actor.h"

namespace nvl {

Actor AbstractMessage::src() const { return Actor(src_); }

} // namespace nvl
