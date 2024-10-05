#pragma once

#include "nvl/macros/Aliases.h"
#include "nvl/tool/BlockCreator.h"
#include "nvl/tool/Tool.h"
#include "nvl/ui/Screen.h"

namespace nvl {

template <U64 N>
class World;

template <U64 N>
class ToolBelt final : AbstractScreen {
public:
    class_tag(ToolBelt, AbstractScreen);
    explicit ToolBelt(Window *window, World<N> *world) : AbstractScreen(window), world(world) {
        tools.push_back(Tool<N>::template get<BlockCreator>(window, world));

        children.push_back(tools[0]);

        on_key_down[Key::B] = [&] {
            index = (index + 1) % tools.size();
            children[0] = tools[index];
        };
    }
    void draw_screen() override {}

private:
    World<N> *world;
    List<Tool<N>> tools;
    U64 index = 0;
};

} // namespace nvl
