#pragma once

#include "nvl/actor/Actor.h"
#include "nvl/message/Destroy.h"
#include "nvl/tool/Tool.h"
#include "nvl/ui/Window.h"
#include "nvl/world/World.h"

namespace nvl {

class BlockRemover final : public Tool<2> {
public:
    class_tag(BlockRemover, Tool<2>);
    explicit BlockRemover(Window *window, World<2> *world) : Tool(window, world) {
        // Same action for dragging and moving
        on_mouse_move[{Mouse::Any}] = on_mouse_move[{}] = [this] {
            const Pos<2> pt = world_->window_to_world(window_->center());
            const Range<Actor> entities = world_->entities(pt);
            hovered_ = entities.empty() ? nullptr : *entities.begin();
        };
        on_mouse_up[Mouse::Left] = [this] {
            if (hovered_) {
                world_->send<Destroy>(nullptr, hovered_, Destroy::Cause::kRemoved);
                hovered_ = nullptr;
            }
        };
    }

    void tick() override {}
    void draw() override {
        if (hovered_) {
            window_->push_view(world_->view());
            hovered_->draw(window_, Color::kLighter);
            window_->pop_view();
        }
    }

private:
    Actor hovered_ = nullptr;
};

} // namespace nvl