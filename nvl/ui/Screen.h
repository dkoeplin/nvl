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
#include "nvl/ui/Scroll.h"

namespace nvl {

struct Screen;
class Window;

abstract class AbstractScreen : Castable<Screen, AbstractScreen, std::shared_ptr<Screen>>::BaseClass {
public:
    class_tag(AbstractScreen);
    explicit AbstractScreen(Window *window) : window_(window) {}

    List<InputEvent> feed_all(const List<InputEvent> &events);
    void tick_all();
    void draw_all();

protected:
    /// Mark the current input event being processed as propagated to other siblings and/or children.
    void propagate_event() { propagated_event_ = true; }

    virtual void tick() = 0;
    virtual void draw() = 0;

    struct ButtonsHash {
        U64 operator()(const Set<Mouse> &buttons) const noexcept { return sip_hash(buttons.values()); }
    };
    Map<Key, std::function<void()>> on_key_up;
    Map<Key, std::function<void()>> on_key_down;
    Map<Mouse, std::function<void()>> on_mouse_up;
    Map<Mouse, std::function<void()>> on_mouse_down;
    Map<Set<Mouse>, std::function<void()>, ButtonsHash> on_mouse_move;
    Map<Scroll, std::function<void()>> on_mouse_scroll;

    List<std::shared_ptr<AbstractScreen>> children_;
    Window *window_;
    bool propagated_event_ = false;
};

struct Screen final : Castable<Screen, AbstractScreen, std::shared_ptr<AbstractScreen>> {
    using Castable::Castable;
    using Castable::get;
    pure std::shared_ptr<AbstractScreen> shared_ptr() const { return ptr_; }
};

} // namespace nvl
