#include "nvl/entity/Entity.h"
#include "nvl/material/Bulwark.h"
#include "nvl/reflect/Backtrace.h"
#include "nvl/tool/ToolBelt.h"
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
    params.gravity_accel = 2;
    auto *world = window.open<World<2>>(params);
    init_world(&window, world);

    window.open<ToolBelt<2>>(world);

    Time prev_tick = Clock::now();
    while (!window.should_close()) {
        if (const Time now = Clock::now(); Duration(now - prev_tick) >= world->kNanosPerTick) {
            prev_tick = now;
            window.tick();
        }
        window.feed();
        window.draw();
    }
}
