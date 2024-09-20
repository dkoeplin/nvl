#include "nvl/message/Message.h"

namespace nvl {

pure Ref<Actor> Message::src() const { return impl_->src; }
pure List<Ref<Actor>> Message::dst() const { return impl_->dst; }

} // namespace nvl
