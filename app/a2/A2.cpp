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
    register_signal_handlers();

    RayWindow window("App", {1000, 1000});
    window.set_mouse_mode(Window::MouseMode::kViewport);
    auto *world = window.open<WorldA2>();
    window.open<PlayerControls>(world);
    window.open<DebugScreen>(world);

    const Duration kNanosPerDraw(3e7); // 30ms, ~33 fps
    const Duration kNanosPerTick(world->kNanosPerTick);
    window.loop(kNanosPerTick, kNanosPerDraw);
}