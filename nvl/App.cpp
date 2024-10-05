#include "nvl/entity/Entity.h"
#include "nvl/tool/ToolBelt.h"
#include "nvl/ui/Window.h"
#include "nvl/world/World.h"
#include "raylib.h"

int main() {
    nvl::Window window("App", {1000, 1000});
    window.open<nvl::World<2>>();
    window.open<nvl::ToolBelt<2>>();

    while (!window.should_close()) {
        window.tick();
        window.draw();
    }
}
