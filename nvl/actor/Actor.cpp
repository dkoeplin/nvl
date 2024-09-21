#include "nvl/actor/Actor.h"

#include "nvl/actor/TickResult.h"

namespace nvl {

TickResult Actor::tick(const List<Message> &messages) { return impl_->tick(messages); }

void Actor::draw(Draw &draw, int64_t highlight) const { impl_->draw(draw, highlight); }

} // namespace nvl
