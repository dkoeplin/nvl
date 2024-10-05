#pragma once

#include <utility>

#include "nvl/data/List.h"
#include "nvl/macros/Abstract.h"
#include "nvl/reflect/Castable.h"
#include "nvl/ui/Key.h"
#include "nvl/ui/Mouse.h"

namespace nvl {

struct InputEvent;

abstract struct AbstractInputEvent
    : Castable<InputEvent, AbstractInputEvent, std::shared_ptr<AbstractInputEvent>>::BaseClass {
    class_tag(AbstractInputEvent);
};

struct InputEvent final : Castable<InputEvent, AbstractInputEvent, std::shared_ptr<AbstractInputEvent>> {};

struct KeyUp final : AbstractInputEvent {
    class_tag(KeyUp, AbstractInputEvent);
    explicit KeyUp(Key key) : key(key) {}
    Key key;
};

struct KeyDown final : AbstractInputEvent {
    class_tag(KeyDown, AbstractInputEvent);
    explicit KeyDown(Key key) : key(key) {}
    Key key;
};

struct MouseUp final : AbstractInputEvent {
    class_tag(MouseUp, AbstractInputEvent);
    explicit MouseUp(Mouse button) : button(button) {}
    Mouse button;
};

struct MouseDown final : AbstractInputEvent {
    class_tag(MouseDown, AbstractInputEvent);
    explicit MouseDown(Mouse button) : button(button) {}
    Mouse button;
};

struct MouseMove final : AbstractInputEvent {
    class_tag(MouseMove, AbstractInputEvent);
    explicit MouseMove(const Set<Mouse> &buttons) : buttons(buttons) {}
    Set<Mouse> buttons;
};

} // namespace nvl
