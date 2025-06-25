#include "a0/tool/BlockCreator.h"

#include "nvl/actor/Actor.h"
#include "nvl/entity/Block.h"
#include "nvl/geo/Tuple.h"
#include "nvl/geo/Volume.h"
#include "nvl/material/Material.h"
#include "nvl/material/TestMaterial.h"
#include "nvl/ui/Color.h"
#include "nvl/ui/Window.h"
#include "nvl/world/World.h"

namespace nvl {

BlockCreator::BlockCreator(AbstractScreen *parent, World<2> *world) : Tool(parent, world) {
    on_mouse_down[Mouse::Left] = [this] {
        if (pending_) {
            world_->reify(std::move(pending_));
            pending_ = nullptr;
        } else {
            const auto color = world_->random.uniform<Color>(0, 255);
            const auto material = Material::get<TestMaterial>(color);
            init_ = world_->window_to_world(window_->center());
            pending_ = std::make_unique<Block<2>>(Pos<2>::zero, Box<2>(init_, init_), material);
        }
    };
    on_mouse_move[{Mouse::Any}] = [this] {
        propagate_event(); // Still allow world coordinates to change on mouse move
        if (pending_) {
            const Pos<2> pt = world_->window_to_world(window_->center());
            const Box<2> box(init_, pt);
            if (world_->entities(box).empty()) {
                pending_ = std::make_unique<Block<2>>(Pos<2>::zero, box, pending_->material());
            }
        }
    };
}

void BlockCreator::draw() {
    if (pending_) {
        window_->push_view(world_->view());
        pending_->draw(window_, Color::kMoreTransparent);
        window_->pop_view();
    }
}

} // namespace nvl
