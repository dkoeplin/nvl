#include "nvl/ui/Window.h"

#include <thread>

namespace nvl {

Window::Window(const std::string &, Tuple<2, I64>) : AbstractScreen(nullptr) { window_ = this; }

void Window::draw() {
    const Time start = Clock::now();
    predraw();
    fwd(this, [](Screen screen) { screen->draw(); });
    postdraw();
    last_draw_time_ = Clock::now() - start;
}

void Window::tick() {
    const Time start = Clock::now();
    bwd(this, [](Screen screen) {
        screen->tick();
        screen->update();
    });
    update();
    last_tick_time_ = Clock::now() - start;
}

void Window::react() {
    List<InputEvent> events = detect_events();
    bwd(this, [&](Screen screen) {
        events.remove_if([&](const InputEvent &event) { return !screen->consume_event(event); });
        screen->react();
        screen->update();
    });
    update();
}

void Window::push_view(const ViewOffset &offset) {
    if (!views_.empty())
        end_view_offset(views_.back());
    views_.push_back(offset);
    set_view_offset(views_.back());
}

void Window::pop_view() {
    if (!views_.empty()) {
        end_view_offset(views_.back());
        views_.pop_back();
    }
    if (!views_.empty()) {
        set_view_offset(views_.back());
    }
}

void Window::loop(const Duration &nanos_per_tick, const Duration &nanos_per_draw) {
    Time prev_tick = Clock::now();
    Time prev_draw = Clock::now();
    while (!should_close()) {
        Time now = Clock::now();
        if (Duration(now - prev_tick) >= nanos_per_tick) {
            prev_tick = now; // Start of most recent tick
            tick();
            react();
        }

        now = Clock::now();
        if (Duration(now - prev_draw) >= nanos_per_draw) {
            prev_draw = now; // Start of most recent draw
            draw();
        }
        now = Clock::now();
        const auto time_to_next_tick = nanos_per_tick - Duration(now - prev_tick);
        const auto time_to_next_draw = nanos_per_draw - Duration(now - prev_draw);
        const auto wait_time = min(time_to_next_tick, time_to_next_draw);
        if (wait_time > 0) {
            std::this_thread::sleep_for(wait_time);
        }
    }
}

} // namespace nvl
