#include "nvl/ui/Window.h"

namespace nvl {

Window::Window(const std::string &, Pos<2>) {}

void Window::tick_all() {
    ticks_ += 1;
    tick();
    for (auto &child : children_) {
        child->tick_all();
    }
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
