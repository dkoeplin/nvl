#pragma once

#include "nvl/data/Map.h"
#include "nvl/data/Set.h"
#include "nvl/data/SipHash.h"
#include "nvl/geo/Pos.h"
#include "nvl/macros/Abstract.h"
#include "nvl/ui/Key.h"
#include "nvl/ui/Mouse.h"

namespace nvl {

class Window;

abstract class UserInterface {
public:
    UserInterface() = default;
    virtual ~UserInterface() = default;
    void update();
    void tick() {
        update();
        query();
    }

    virtual void query() {}
    virtual void on_mouse_move() {}
    virtual void on_exit() {}
    virtual void draw(Window &window) = 0;

    pure bool is_pressed(const Key key) const { return pressed_keys.has(key); }
    pure bool is_pressed(const Mouse mouse) const { return pressed_mouse.has(mouse); }

protected:
    struct ButtonHash {
        pure U64 operator()(const Set<Mouse> &buttons) const noexcept { return sip_hash(buttons.values()); }
    };

    Map<Key, std::function<void()>> on_key_down;
    Map<Key, std::function<void()>> on_key_up;
    Map<Mouse, std::function<void()>> on_mouse_down;
    Map<Mouse, std::function<void()>> on_mouse_up;
    Map<Set<Mouse>, std::function<void()>, ButtonHash> on_mouse_drag;

    Set<Key> pressed_keys;
    Set<Mouse> pressed_mouse;
    Pos<2> curr_mouse;
    Pos<2> prev_mouse;
};

} // namespace nvl
