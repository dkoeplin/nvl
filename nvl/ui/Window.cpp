#include "nvl/ui/Window.h"

namespace nvl {

Window::Window(const std::string &, Tuple<2, I64>) : AbstractScreen(nullptr) { window_ = this; }

void Window::draw() {
    const Time start = Clock::now();
    predraw();
    AbstractScreen::fwd(this, [](Screen screen) { screen->draw(); });
    postdraw();
    last_draw_time_ = Clock::now() - start;
}

void Window::tick() {
    const Time start = Clock::now();
    AbstractScreen::bwd(this, [](Screen screen) {
        screen->tick();
        screen->update();
    });
    update();
    last_tick_time_ = Clock::now() - start;
}

void Window::react() {
    List<InputEvent> events = detect_events();
    AbstractScreen::bwd(this, [&](Screen screen) {
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

} // namespace nvl
