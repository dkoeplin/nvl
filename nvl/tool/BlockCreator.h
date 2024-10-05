#pragma once

#include "nvl/actor/Actor.h"
#include "nvl/data/Maybe.h"
#include "nvl/draw/Color.h"
#include "nvl/entity/Block.h"
#include "nvl/geo/Box.h"
#include "nvl/geo/Pos.h"
#include "nvl/macros/Aliases.h"
#include "nvl/material/Material.h"
#include "nvl/material/TestMaterial.h"
#include "nvl/tool/Tool.h"
#include "nvl/ui/Window.h"

namespace nvl {

class BlockCreator final : public AbstractTool<2> {
public:
    class_tag(BlockCreator, AbstractTool<2>);
    explicit BlockCreator(Window *window, World<2> *world) : AbstractTool(window, world) {
        on_mouse_down[Mouse::Left] = [&] {
            auto color = world_->random.uniform<Color>(0, 255);
            auto material = Material::get<TestMaterial>(color);
            pending_ = Actor::get<Block<2>>(window->mouse_coord(), Box<2>::unit(Pos<2>::zero), material);
        };
        on_mouse_move[{Mouse::Left}] = [&] {
            if (pending_.has_value()) {
                const Block<2> &block = *pending_->dyn_cast<Block<2>>();
                const Pos<2> init = block.loc();
                if (world_->entities({init, window->mouse_coord()}).empty()) {
                    const Material material = block.material();
                    pending_ = Actor::get<Block<2>>(init, Box(Pos<2>::zero, window->mouse_coord() - init), material);
                }
            }
        };
        on_mouse_up[Mouse::Left] = [&] {
            if (pending_.has_value()) {
                world_->reify(*pending_);
            }
            pending_ = None;
        };
    }

    void tick() override {}
    void draw() override {
        if (pending_.has_value()) {
            (*pending_)->draw(*window_, {.alpha = Color::kLighter});
        }
    }

private:
    Maybe<Actor> pending_;
};

} // namespace nvl
