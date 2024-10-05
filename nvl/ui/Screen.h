#pragma once

#include "nvl/data/List.h"
#include "nvl/data/Map.h"
#include "nvl/data/Set.h"
#include "nvl/data/SipHash.h"
#include "nvl/geo/Pos.h"
#include "nvl/macros/Abstract.h"
#include "nvl/reflect/Castable.h"
#include "nvl/ui/InputEvent.h"
#include "nvl/ui/Key.h"
#include "nvl/ui/Mouse.h"

namespace nvl {

struct Screen;
class Window;

abstract class AbstractScreen : Castable<Screen, AbstractScreen, std::shared_ptr<Screen>>::BaseClass {
public:
    class_tag(AbstractScreen);
    explicit AbstractScreen(Window *window) : window_(window) {}
    List<InputEvent> tick_all(const List<InputEvent> &events);
    void draw_all();

    virtual void tick() = 0;
    virtual void draw() = 0;

protected:
    struct ButtonsHash {
        U64 operator()(const Set<Mouse> &buttons) const noexcept { return sip_hash(buttons.values()); }
    };
    Map<Key, std::function<void()>> on_key_up;
    Map<Key, std::function<void()>> on_key_down;
    Map<Mouse, std::function<void()>> on_mouse_up;
    Map<Mouse, std::function<void()>> on_mouse_down;
    Map<Set<Mouse>, std::function<void()>, ButtonsHash> on_mouse_move;

    List<Screen> children_;
    Window *window_;
};

struct Screen final : Castable<Screen, AbstractScreen, std::shared_ptr<AbstractScreen>> {};

} // namespace nvl
