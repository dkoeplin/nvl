#pragma once

#include "nvl/data/Maybe.h"
#include "nvl/draw/Color.h"
#include "nvl/entity/Block.h"
#include "nvl/geo/Box.h"
#include "nvl/geo/Pos.h"
#include "nvl/macros/Aliases.h"
#include "nvl/material/Material.h"
#include "nvl/material/TestMaterial.h"
#include "nvl/tool/Tool.h"

namespace nvl {

class BlockCreator final : public AbstractTool<2> {
public:
    class_tag(BlockCreator, AbstractActor<2>);
    explicit BlockCreator(World<2> *world) : AbstractTool(world) {
        on_mouse_down[Mouse::Left] = [&] {
            auto color = world_->random.uniform<Color>(0, 255);
            auto material = Material::get<TestMaterial>(color);
            pending_ = Actor::get<Block<2>>(curr_mouse, Box<2>::unit(Pos<2>::zero), material);
        };
        on_mouse_drag[{Mouse::Left}] = [&] {
            if (pending_.has_value()) {
                const Block<2> &block = *pending_->dyn_cast<Block<2>>();
                const Pos<2> init = block.loc();
                if (world_->entities({init, curr_mouse}).empty()) {
                    const Material material = block.material();
                    pending_ = Actor::get<Block<2>>(init, Box(Pos<2>::zero, curr_mouse - init), material);
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

    void draw(Window &window) override {

    }

private:
    Maybe<Actor> pending_;
};

} // namespace nvl
