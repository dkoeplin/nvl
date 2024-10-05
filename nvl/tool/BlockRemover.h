#pragma once

#include "nvl/actor/Actor.h"
#include "nvl/data/Maybe.h"
#include "nvl/message/Destroy.h"
#include "nvl/tool/Tool.h"
#include "nvl/ui/Window.h"
#include "nvl/world/World.h"

namespace nvl {

class BlockRemover final : public AbstractTool<2> {
public:
    class_tag(BlockRemover, AbstractTool<2>);
    explicit BlockRemover(Window *window, World<2> *world) : AbstractTool(window, world) {
        // Same action for dragging and moving
        on_mouse_move[{Mouse::Any}] = on_mouse_move[{}] = [&] {
            const Pos<2> pos = world_->mouse_to_world(window_->mouse_coord());
            Range<Actor> entities = world_->entities(pos);
            hovered_ = entities.empty() ? None : Some(*entities.begin());
        };
        on_mouse_up[Mouse::Left] = [&] {
            if (hovered_.has_value()) {
                world->send<Destroy>(nullptr, *hovered_);
                hovered_ = None;
            }
        };
    }

    void tick() override {}
    void draw() override {
        if (hovered_.has_value()) {
            (*hovered_)->draw(*window_, {.scale = Color::kLighter});
        }
    }

private:
    Maybe<Actor> hovered_ = None;
};

} // namespace nvl