#pragma once

#include "nvl/macros/Abstract.h"
#include "nvl/reflect/Castable.h"
#include "nvl/ui/Key.h"
#include "nvl/ui/Mouse.h"
#include "nvl/ui/Scroll.h"

namespace nvl {

struct InputEvent;

abstract struct AbstractInputEvent
    : Castable<InputEvent, AbstractInputEvent, std::shared_ptr<AbstractInputEvent>>::BaseClass {
    class_tag(AbstractInputEvent);
    pure virtual std::string to_string() const = 0;
};

struct InputEvent final : Castable<InputEvent, AbstractInputEvent, std::shared_ptr<AbstractInputEvent>> {
    using Castable::Castable;
    using Castable::get;
    pure std::string to_string() const { return ptr_->to_string(); }
};

struct KeyUp final : AbstractInputEvent {
    class_tag(KeyUp, AbstractInputEvent);
    explicit KeyUp(const Key key) : key(key) {}
    pure std::string to_string() const override { return "KeyUp(" + key.to_string() + ")"; }
    Key key;
};

struct KeyDown final : AbstractInputEvent {
    class_tag(KeyDown, AbstractInputEvent);
    explicit KeyDown(const Key key) : key(key) {}
    pure std::string to_string() const override { return "KeyDown(" + key.to_string() + ")"; }
    Key key;
};

struct MouseUp final : AbstractInputEvent {
    class_tag(MouseUp, AbstractInputEvent);
    explicit MouseUp(const Mouse button) : button(button) {}
    pure std::string to_string() const override { return "MouseUp(" + button.to_string() + ")"; }
    Mouse button;
};

struct MouseDown final : AbstractInputEvent {
    class_tag(MouseDown, AbstractInputEvent);
    explicit MouseDown(const Mouse button) : button(button) {}
    pure std::string to_string() const override { return "MouseDown(" + button.to_string() + ")"; }
    Mouse button;
};

struct MouseMove final : AbstractInputEvent {
    class_tag(MouseMove, AbstractInputEvent);
    explicit MouseMove(const Set<Mouse> &buttons) : buttons(buttons) {}
    pure std::string to_string() const override {
        std::stringstream ss;
        ss << "MouseMove(" << buttons << ")";
        return ss.str();
    }
    Set<Mouse> buttons;
};

struct MouseScroll final : AbstractInputEvent {
    class_tag(MouseScroll, AbstractInputEvent);
    explicit MouseScroll(const Scroll &scroll) : scroll(scroll) {}

    pure std::string to_string() const override { return "MouseScroll(" + scroll.to_string() + ")"; }

    Scroll scroll;
};

inline std::ostream &operator<<(std::ostream &os, const AbstractInputEvent &event) { return os << event.to_string(); }
inline std::ostream &operator<<(std::ostream &os, const InputEvent &event) { return os << event.to_string(); }

} // namespace nvl
