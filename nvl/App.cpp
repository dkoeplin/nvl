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

int main() {
    nvl::RayWindow window("App", {1000, 1000});
    window.set_mouse_mode(nvl::Window::MouseMode::kViewport);
    auto *world = window.open<nvl::World<2>>();
    nvl::init_world(&window, world);

    window.open<nvl::ToolBelt<2>>(world);

    while (!window.should_close()) {
        window.tick();
        window.draw();
    }
}
