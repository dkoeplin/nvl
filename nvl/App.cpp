#include "nvl/draw/Window.h"
#include "nvl/entity/Entity.h"
#include "nvl/tool/BlockCreator.h"
#include "nvl/ui/UserInterface.h"
#include "nvl/world/World.h"
#include "raylib.h"

namespace nvl {

class Interface : UserInterface {
public:
    Interface() : UserInterface() {
        tools_ = {
            Tool<2>::get<BlockCreator<2>>(),
        };
    }

private:
    pure Tool<2> tool() const { return tools_[current_]; }
    List<Tool<2>> tools_;
    U64 current_ = 0;
};
} // namespace nvl

int main() {
    nvl::Window window("App", {1000, 1000});
    nvl::World<2> world;
    nvl::Interface interface;
    while (!window.should_close()) {
        BeginDrawing();
        window.draw();
        world.draw(window, nvl::Color::kNormal);
        EndDrawing();
    }
}
