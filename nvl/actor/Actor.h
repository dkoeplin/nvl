#pragma once

#include "nvl/actor/Status.h"
#include "nvl/data/List.h"
#include "nvl/data/SipHash.h"
#include "nvl/macros/Abstract.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/Pure.h"
#include "nvl/reflect/Castable.h"
#include "nvl/ui/Color.h"

namespace nvl {

struct Actor;
class Window;
struct Message;

abstract struct AbstractActor : Castable<Actor, AbstractActor>::BaseClass {
    class_tag(AbstractActor);
    virtual Status tick(const List<Message> &messages) = 0;
    virtual void draw(Window *window, const Color &scale) const = 0;
};

struct Actor final : Castable<Actor, AbstractActor> {
    using Castable::Castable;
    using Castable::get;
    pure bool operator==(const Actor &rhs) const { return ptr() == rhs.ptr(); }
    pure bool operator!=(const Actor &rhs) const { return ptr() != rhs.ptr(); }
};

} // namespace nvl

template <>
struct std::hash<nvl::Actor> {
    pure U64 operator()(const nvl::Actor &actor) const noexcept { return sip_hash(actor.ptr()); }
};
