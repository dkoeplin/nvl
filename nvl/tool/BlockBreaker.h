#pragma once

#include "nvl/message/Hit.h"
#include "nvl/tool/Tool.h"
#include "nvl/ui/Window.h"
#include "nvl/world/World.h"

namespace nvl {

class BlockBreaker final : public Tool<2> {
public:
    class_tag(BlockBreaker, Tool<2>);
    explicit BlockBreaker(Window *window, World<2> *world) : Tool(window, world) {
        on_mouse_down[Mouse::Left] = on_mouse_move[{Mouse::Left}] = [this] {
            const Pos<2> pt = world_->window_to_world(window_->center());
            const Box<2> box(pt - radius_, pt + radius_);
            const Range<Actor> entities = world_->entities(box);
            world_->send<Hit<2>>(nullptr, entities, box, /*strength*/ 1);
        };
        on_mouse_scroll[Scroll::kVertical] = [this] {
            const auto dist = window_->scroll_y();
            radius_ = std::clamp<I64>(radius_ + dist, 1, 100);
        };
    }

    void tick() override {}
    void draw() override {
        static constexpr Color color(255, 0, 0, 32);
        const Pos<2> center = window_->center();
        const Box<2> box(center - radius_, center + radius_);
        window_->fill_box(color, box);
    }

private:
    I64 radius_ = 20;
};

} // namespace nvl
