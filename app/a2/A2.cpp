#include <thread>

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

    const Duration kNanosPerDraw(3e7); // 30ms, ~33 fps
    const Duration kNanosPerTick(world->kNanosPerTick);

    Time prev_tick = Clock::now();
    Time prev_draw = Clock::now();
    while (!window.should_close()) {
        Time now = Clock::now();
        if (Duration(now - prev_tick) >= kNanosPerTick) {
            prev_tick = now; // Start of most recent tick
            window.tick();
            window.react();
        }

        now = Clock::now();
        if (Duration(now - prev_draw) >= kNanosPerDraw) {
            prev_draw = now; // Start of most recent draw
            window.draw();
        }
        now = Clock::now();
        const auto time_to_next_tick = kNanosPerTick - Duration(now - prev_tick);
        const auto time_to_next_draw = kNanosPerDraw - Duration(now - prev_draw);
        const auto wait_time = min(time_to_next_tick, time_to_next_draw);
        if (wait_time > 0) {
            std::this_thread::sleep_for(wait_time);
        }
    }
}