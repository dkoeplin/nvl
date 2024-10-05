#include "nvl/ui/Screen.h"

#include "Window.h"

namespace nvl {

List<InputEvent> AbstractScreen::tick_all(const List<InputEvent> &events) {
    List<InputEvent> forwarded;
    for (const InputEvent &event : events) {
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
        }
        if (func != nullptr) {
            (*func)();
        } else {
            forwarded.push_back(event);
        }
    }
    for (auto iter = children_.begin(); !forwarded.empty() && iter != children_.end(); ++iter) {
        forwarded = (*iter)->tick_all(forwarded);
    }
    return forwarded;
}

void AbstractScreen::draw_all() {
    draw();
    for (auto &child : children_) {
        child->draw_all();
    }
}

} // namespace nvl
