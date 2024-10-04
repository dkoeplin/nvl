#include "nvl/ui/UserInterface.h"

#include "raylib.h"

namespace nvl {

void UserInterface::tick() {
    for (const auto &[key, action] : on_key_down) {
        if (IsKeyPressed(key))
            action();
    }
    for (const auto &[key, action] : on_key_up) {
        if (IsKeyReleased(key))
            action();
    }
    for (const auto &[but, action] : on_mouse_down) {
        if (IsMouseButtonPressed(but))
            action();
    }
    for (const auto &[but, action] : on_mouse_up) {
        if (IsMouseButtonReleased(but))
            action();
    }
}

} // namespace nvl
