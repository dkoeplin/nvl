#pragma once

#include "nvl/actor/Actor.h"
#include "nvl/draw/Color.h"
#include "nvl/entity/Block.h"
#include "nvl/geo/Box.h"
#include "nvl/geo/Pos.h"
#include "nvl/material/Material.h"
#include "nvl/material/TestMaterial.h"
#include "nvl/tool/Tool.h"
#include "nvl/ui/Window.h"

namespace nvl {

class BlockCreator final : public Tool<2> {
public:
    class_tag(BlockCreator, Tool<2>);
    explicit BlockCreator(Window *window, World<2> *world) : Tool(window, world) {
        on_mouse_down[Mouse::Left] = [this] {
            const auto color = world_->random.uniform<Color>(0, 255);
            const auto material = Material::get<TestMaterial>(color);
            init_ = world_->window_to_world(window_->center());
            pending_ = std::make_unique<Block<2>>(Pos<2>::zero, Box(init_, init_), material);
        };
        on_mouse_move[{Mouse::Left}] = [this] {
            if (pending_) {
                const Pos<2> pt = world_->window_to_world(window_->center());
                const Box box(init_, pt);
                if (world_->entities(box).empty()) {
                    pending_ = std::make_unique<Block<2>>(Pos<2>::zero, box, pending_->material());
                }
            }
        };
        on_mouse_up[Mouse::Left] = [this] {
            if (pending_) {
                world_->reify(std::move(pending_));
                pending_ = nullptr;
            }
        };
    }

    void tick() override {}
    void draw() override {
        if (pending_) {
            const auto offset = Window::Offset::Absolute(window_, world_->view());
            pending_->draw(window_, {.alpha = Color::kLighter});
        }
    }

private:
    std::unique_ptr<Block<2>> pending_ = nullptr;
    Pos<2> init_ = Pos<2>::zero;
};

} // namespace nvl
