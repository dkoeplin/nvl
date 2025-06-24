#pragma once

#include "nvl/data/List.h"
#include "nvl/data/Map.h"
#include "nvl/data/Set.h"
#include "nvl/data/SipHash.h"
#include "nvl/macros/Abstract.h"
#include "nvl/macros/Pure.h"
#include "nvl/reflect/CastableShared.h"
#include "nvl/ui/InputEvent.h"
#include "nvl/ui/Key.h"
#include "nvl/ui/Mouse.h"
#include "nvl/ui/Scroll.h"

namespace nvl {

struct Screen;
class Window;

abstract class AbstractScreen : public CastableShared<Screen, AbstractScreen>::BaseClass {
public:
    class_tag(AbstractScreen);

    /// Traverses all screens below the starting screen and calls visit on each.
    static void fwd(AbstractScreen *top, const std::function<void(Screen)> &visit);

    /// Traverses all screens in reverse below the starting screen and calls visit on each.
    static void bwd(AbstractScreen *top, const std::function<void(Screen)> &visit);

    explicit AbstractScreen(AbstractScreen *parent);

    template <typename T, typename... Args>
    T *open(Args &&...args);

    pure bool closed() const { return closed_; }

    pure bool event_propagated() const { return propagated_event_; }

    /// Draws this screen.
    virtual void draw() = 0;

    /// Runs any per-tick operations.
    virtual void tick() = 0;

    /// Generic method for responding to user input.
    /// For most use cases, register responses in on_[key|mouse]_[up|down], on_mouse_move, on_mouse_scroll.
    virtual void react() {}

    /// Default method for reacting to events using registered event handlers.
    /// Returns true if the event should be propagated, false otherwise.
    bool consume_event(const InputEvent &event);

    void update();

    pure Window *window() const { return window_; }

protected:
    /// Mark the current input event being processed as propagated to other siblings and/or children.
    void propagate_event() { propagated_event_ = true; }

    /// Mark this window as being closed. It will be deleted at the end of the completed tick.
    void close() { closed_ = true; }

    struct ButtonsHash {
        U64 operator()(const Set<Mouse> &buttons) const noexcept { return sip_hash(buttons.values()); }
    };
    Map<Key, std::function<void()>> on_key_up;
    Map<Key, std::function<void()>> on_key_down;
    Map<Mouse, std::function<void()>> on_mouse_up;
    Map<Mouse, std::function<void()>> on_mouse_down;
    Map<Set<Mouse>, std::function<void()>, ButtonsHash> on_mouse_move;
    Map<Scroll, std::function<void()>> on_mouse_scroll;

    List<Screen> children_;
    AbstractScreen *parent_;
    Window *window_;
    bool propagated_event_ = false;
    bool closed_ = false;
};

struct Screen final : CastableShared<Screen, AbstractScreen> {
    using CastableShared::CastableShared;
    using CastableShared::get;
};

template <typename T, typename... Args>
T *AbstractScreen::open(Args &&...args) {
    // Add a Screen to the children and return the dynamically casted result.
    // Note that this isn't a pointer to the children list element itself, so it should be stable.
    children_.push_back(Screen::get<T>(this, std::forward<Args>(args)...));
    return children_.back().dyn_cast<T>();
}

} // namespace nvl
