#include "nvl/entity/Entity.h"
#include "nvl/tool/ToolBelt.h"
#include "nvl/ui/RayWindow.h"
#include "nvl/world/World.h"

namespace nvl {

void init_world(Window *window, World<2> *world) {
    const Material bulwark = Material::get<Bulwark>();
    const Pos<2> min = {0, window->height() - 50};
    const Pos<2> max = {window->width(), window->height()};
    const Box<2> base(min, max);
    world->spawn<Block<2>>(Pos<2>::zero, base, bulwark);
}

} // namespace nvl

using nvl::RayWindow;
using nvl::ToolBelt;
using nvl::Window;
using nvl::World;

int main() {
    RayWindow window("App", {1000, 1000});
    window.set_mouse_mode(Window::MouseMode::kViewport);
    auto *world = window.open<World<2>>();
    init_world(&window, world);

    window.open<ToolBelt<2>>(world);

    using Clock = std::chrono::steady_clock;
    std::chrono::time_point<std::chrono::steady_clock> prev_tick = Clock::now();
    while (!window.should_close()) {
        const auto now = Clock::now();
        if (std::chrono::duration_cast<std::chrono::nanoseconds>(now - prev_tick).count() < World<2>::kNanosPerTick) {
            prev_tick = now;
            window.tick();
        }

        window.draw();
    }
}
