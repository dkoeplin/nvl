#include "a0/tool/ToolBelt.h"
#include "nvl/entity/Block.h"
#include "nvl/entity/Entity.h"
#include "nvl/material/Bulwark.h"
#include "nvl/reflect/Backtrace.h"
#include "nvl/ui/RayWindow.h"
#include "nvl/world/World.h"

namespace nvl {

void init_world(const Window *window, World<2> *world) {
    const Material bulwark = Material::get<Bulwark>();
    const Pos<2> min = {0, window->height() - 50};
    const Pos<2> max = {window->width(), window->height()};
    const Box<2> base(min, max);
    world->spawn<Block<2>>(Pos<2>::zero, base, bulwark);
}

} // namespace nvl

using nvl::Clock;
using nvl::Duration;
using nvl::RayWindow;
using nvl::Time;
using nvl::ToolBelt;
using nvl::Window;
using nvl::World;

int main() {
    nvl::register_signal_handlers();

    RayWindow window("App", {1000, 1000});
    window.set_mouse_mode(Window::MouseMode::kViewport);
    World<2>::Params params;
    params.maximum_y = 5000;
    auto *world = window.open<World<2>>(params);
    init_world(&window, world);

    window.open<ToolBelt<2>>(world);

    const Duration kNanosPerTick(world->kNanosPerTick);
    const Duration kNanosPerDraw(3e7);
    window.loop(kNanosPerTick, kNanosPerDraw);
}
