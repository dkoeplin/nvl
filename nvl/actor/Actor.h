#pragma once

#include "nvl/data/SipHash.h"
#include "nvl/geo/Box.h"
#include "nvl/macros/Abstract.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/Pure.h"
#include "nvl/reflect/Castable.h"

namespace nvl {

class Draw;
class Message;
class TickResult;

abstract struct AbstractActor {
    class_tag(AbstractActor);
    virtual ~AbstractActor() = default;

    virtual TickResult tick(const List<Message> &messages) = 0;
    virtual void draw(Draw &draw, I64 highlight) const = 0;
};

struct Actor : Castable<AbstractActor> {
    friend struct std::hash<Actor>;

    TickResult tick(const List<Message> &messages);
    void draw(Draw &draw, I64 highlight) const;

    pure bool operator==(const Actor &rhs) const { return this == &rhs; }
    pure bool operator!=(const Actor &rhs) const { return this != &rhs; }
};

} // namespace nvl

template <>
struct std::hash<nvl::Actor> {
    pure expand U64 operator()(const nvl::Actor &actor) const noexcept { return sip_hash(actor.impl_.get()); }
};