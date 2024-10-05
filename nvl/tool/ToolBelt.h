#pragma once

#include "nvl/macros/Aliases.h"
#include "nvl/tool/BlockBreaker.h"
#include "nvl/tool/BlockCreator.h"
#include "nvl/tool/BlockRemover.h"
#include "nvl/tool/Tool.h"
#include "nvl/ui/Screen.h"

namespace nvl {

template <U64 N>
class World;

template <U64 N>
class ToolBelt final : AbstractScreen {
public:
    class_tag(ToolBelt, AbstractScreen);
    explicit ToolBelt(Window *window, World<N> *world) : AbstractScreen(window), world_(world) {
        tools_ = {
            Screen::get<BlockCreator>(window, world), // Creator
            Screen::get<BlockBreaker>(window, world), // Breaker
            Screen::get<BlockRemover>(window, world)  // Remover
        };

        children_.push_back(tools_[0]);

        on_key_down[Key::B] = [this] {
            index_ = (index_ + 1) % tools_.size();
            children_[0] = tools_[index_];
            cooldown_ = 100;
        };
    }
    void tick() override {
        if (cooldown_ > 0)
            --cooldown_;
    }
    void draw() override {}

private:
    World<N> *world_;
    List<Screen> tools_;
    U64 index_ = 0;
    U64 cooldown_ = 0;
}; // namespace nvl

} // namespace nvl
