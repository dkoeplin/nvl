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

    // constexpr I64 kNanosPerDraw = 1e8;

    Time prev_tick = Clock::now();
    // Time prev_draw = Clock::now();
    while (!window.should_close()) {
        const Time now = Clock::now();
        if (Duration(now - prev_tick) >= world->kNanosPerTick) {
            prev_tick = now;
            window.tick();
        }
        window.react();
        // if (Duration(now - prev_draw) >= kNanosPerDraw) {
        //     prev_draw = now;
        window.draw();
        //}
    }
}
