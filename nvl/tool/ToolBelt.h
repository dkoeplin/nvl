#pragma once

#include "nvl/macros/Aliases.h"
#include "nvl/reflect/ClassTag.h"
#include "nvl/tool/BlockBreaker.h"
#include "nvl/tool/BlockCreator.h"
#include "nvl/tool/BlockRemover.h"
#include "nvl/tool/Tool.h"
#include "nvl/ui/Screen.h"

namespace nvl {

template <U64 N>
class World;

template <U64 N>
class ToolBelt final : public AbstractScreen {
public:
    class_tag(ToolBelt, AbstractScreen);
    explicit ToolBelt(AbstractScreen *parent, World<N> *world) : AbstractScreen(parent), world_(world) {
        tools_ = {
            Screen::get<BlockCreator>(parent, world), // Creator
            Screen::get<BlockBreaker>(parent, world), // Breaker
            Screen::get<BlockRemover>(parent, world)  // Remover
        };

        children_.push_back(tools_[0]);

        on_key_down[Key::B] = [this] {
            index_ = (index_ + 1) % tools_.size();
            children_[0] = tools_[index_];
            cooldown_ = 60;
        };
    }
    void tick() override {
        if (cooldown_ > 0)
            --cooldown_;
    }
    void draw() override {
        if (cooldown_ > 0) {
            const U64 alpha = std::min<U64>(255, cooldown_ * 10);
            const Color color(20, 20, 20, alpha);
            const std::string_view name = ClassTag::get(*children_[0]).name;
            const Pos<2> pos(window_->width() / 2, window_->height() - 35);
            window_->text(color, pos, 30, name);
        }
    }

private:
    World<N> *world_;
    List<Screen> tools_;
    U64 index_ = 0;
    U64 cooldown_ = 0;
};

} // namespace nvl
