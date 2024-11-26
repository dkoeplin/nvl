#include "nvl/ui/Screen.h"

#include "nvl/ui/Window.h"

namespace nvl {

void AbstractScreen::fwd(AbstractScreen *top, const std::function<void(Screen)> &visit) {
    List<Screen> worklist(top->children_.rrange());
    while (!worklist.empty()) {
        Screen screen = worklist.back();
        worklist.pop_back();
        visit(screen);
        worklist.append(screen->children_.rrange()); // reversed
    }
}

void AbstractScreen::bwd(AbstractScreen *top, const std::function<void(Screen)> &visit) {
    Set<AbstractScreen *> visited;
    List<Screen> worklist = top->children_;
    while (!worklist.empty()) {
        Screen screen = worklist.back();
        if (!visited.has(screen.ptr())) {
            visited.insert(screen.ptr());
            worklist.append(screen->children_.range());
        } else {
            worklist.pop_back();
            visit(screen);
        }
    }
}

AbstractScreen::AbstractScreen(AbstractScreen *parent) : parent_(parent) {
    window_ = parent_ ? parent_->window_ : nullptr;
}

bool AbstractScreen::consume_event(const InputEvent &event) {
    std::function<void()> *func = nullptr;
    if (auto *key_up = event.dyn_cast<KeyUp>()) {
        func = on_key_up.get(key_up->key);
        func = func ? func : on_key_up.get(Key::Any);
    } else if (auto *key_down = event.dyn_cast<KeyDown>()) {
        func = on_key_down.get(key_down->key);
        func = func ? func : on_key_down.get(Key::Any);
    } else if (auto *mouse_up = event.dyn_cast<MouseUp>()) {
        func = on_mouse_up.get(mouse_up->button);
        func = func ? func : on_mouse_up.get(Mouse::Any);
    } else if (auto *mouse_down = event.dyn_cast<MouseDown>()) {
        func = on_mouse_down.get(mouse_down->button);
        func = func ? func : on_mouse_down.get(Mouse::Any);
    } else if (auto *mouse_move = event.dyn_cast<MouseMove>()) {
        func = on_mouse_move.get(mouse_move->buttons);
        if (!window_->pressed_mouse().empty()) {
            func = func ? func : on_mouse_move.get({Mouse::Any});
        }
    } else if (auto *mouse_scroll = event.dyn_cast<MouseScroll>()) {
        func = on_mouse_scroll.get(mouse_scroll->scroll);
    }
    propagated_event_ = (func == nullptr);
    if (func != nullptr) {
        (*func)();
    }
    return propagated_event_;
}

void AbstractScreen::update() {
    children_.remove_if([](Screen child) { return child->closed(); });
}

} // namespace nvl
