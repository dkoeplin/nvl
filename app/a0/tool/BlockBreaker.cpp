#include "a0/tool/BlockBreaker.h"

#include "nvl/entity/Block.h"
#include "nvl/message/Hit.h"
#include "nvl/ui/Window.h"
#include "nvl/world/World.h"

namespace nvl {

BlockBreaker::BlockBreaker(AbstractScreen *parent, World<2> *world) : Tool(parent, world) {
    on_mouse_down[Mouse::Left] = on_mouse_move[{Mouse::Left}] = [this] {
        const Pos<2> pt = world_->window_to_world(window_->center());
        const Box<2> box(pt - radius_, pt + radius_);
        const Set<Actor> entities = world_->entities(box);
        world_->send<Hit<2>>(nullptr, entities.values(), box, /*strength*/ 1);
    };
    on_mouse_scroll[Scroll::kVertical] = [this] {
        const auto dist = window_->scroll_y();
        radius_ = std::clamp<I64>(radius_ + dist, 1, 100);
    };
}

void BlockBreaker::draw() {
    static constexpr Color color(255, 0, 0, 32);
    const Pos<2> center = window_->center();
    const Box<2> box(center - radius_, center + radius_);
    window_->fill_box(color, box);
}

} // namespace nvl
