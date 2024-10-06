#include "nvl/entity/Entity.h"
#include "nvl/tool/ToolBelt.h"
#include "nvl/ui/RayWindow.h"
#include "nvl/world/World.h"

int main() {
    nvl::RayWindow window("App", {1000, 1000});
    auto *world = window.open<nvl::World<2>>();
    window.open<nvl::ToolBelt<2>>(world);

    while (!window.should_close()) {
        window.tick();
        window.draw();
    }
}
