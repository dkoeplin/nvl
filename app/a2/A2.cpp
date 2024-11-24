#include "a2/ui/DebugScreen.h"
#include "a2/ui/PlayerControls.h"
#include "a2/world/WorldA2.h"
#include "nvl/reflect/Backtrace.h"
#include "nvl/ui/RayWindow.h"
#include "nvl/ui/Screen.h"
#include "nvl/ui/Window.h"

using namespace nvl;
using namespace a2;

int main() {
    nvl::register_signal_handlers();

    RayWindow window("App", {1000, 1000});
    window.set_mouse_mode(Window::MouseMode::kViewport);
    auto *world = window.open<WorldA2>();
    window.open<PlayerControls>(world);
    window.open<DebugScreen>(world);

    Time prev_tick = Clock::now();
    while (!window.should_close()) {
        if (const Time now = Clock::now(); Duration(now - prev_tick) >= world->kNanosPerTick) {
            prev_tick = now;
            window.tick_all();
        }
        window.feed();
        window.draw();
    }
}
