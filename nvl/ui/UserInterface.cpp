#include "nvl/ui/UserInterface.h"

#include "nvl/data/List.h"
#include "raylib.h"

namespace nvl {

void UserInterface::update() {
    List<Key> key_pressed;
    List<Key> key_released;
    List<Mouse> mouse_pressed;
    List<Mouse> mouse_released;
    while (auto key = GetKeyPressed()) {
        key_pressed.emplace_back(static_cast<Key::Value>(key));
    }
    for (auto button : Mouse::kButtons) {
        if (IsMouseButtonPressed(button))
            mouse_pressed.emplace_back(button);
        if (IsMouseButtonReleased(button))
            mouse_released.emplace_back(button);
    }
    pressed_keys.insert(key_pressed.range());
    for (auto key : pressed_keys) {
        if (IsKeyReleased(key))
            key_released.push_back(key);
    }

    pressed_keys.remove(key_released.range());
    pressed_mouse.insert(mouse_pressed.range());
    pressed_mouse.remove(mouse_released.range());

    prev_mouse = curr_mouse;
    curr_mouse = {GetMouseX(), GetMouseY()};

    for (auto key : key_pressed) {
        if (const auto *func = on_key_down.get(key))
            (*func)();
    }
    for (auto key : key_released) {
        if (const auto *func = on_key_up.get(key))
            (*func)();
    }
    for (auto button : mouse_pressed) {
        if (const auto *func = on_mouse_down.get(button))
            (*func)();
    }
    for (auto button : mouse_released) {
        if (const auto *func = on_mouse_up.get(button))
            (*func)();
    }
    if (prev_mouse != curr_mouse) {
        if (pressed_mouse.empty()) {
            on_mouse_move();
        } else if (const auto *func = on_mouse_drag.get(pressed_mouse)) {
            (*func)();
        }
    }
}

} // namespace nvl
