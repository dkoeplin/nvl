#pragma once

#include "nvl/data/Map.h"

namespace nvl {

class UserInterface {
public:
    UserInterface() = default;
    void tick();

protected:
    Map<int, std::function<void()>> on_key_down;
    Map<int, std::function<void()>> on_key_up;
    Map<int, std::function<void()>> on_mouse_down;
    Map<int, std::function<void()>> on_mouse_up;
};

} // namespace nvl
