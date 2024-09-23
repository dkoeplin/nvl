#pragma once

#include "nvl/actor/Status.h"
#include "nvl/data/List.h"
#include "nvl/macros/Abstract.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/Pure.h"
#include "nvl/reflect/Castable.h"

namespace nvl {

class Draw;
struct Actor;
struct Message;
struct Status;

abstract struct AbstractActor : Castable<Actor, AbstractActor>::BaseClass {
    class_tag(AbstractActor);
    virtual void tick(const List<Message> &messages) = 0;
    virtual void draw(Draw &draw, I64 highlight) = 0;

    pure Status status() const { return status_; }

protected:
    Status status_ = Status::None;
};

struct Actor final : Castable<Actor, AbstractActor> {
    using Castable::Castable;
    using Castable::get;

    pure bool operator==(const Actor &rhs) const { return this == &rhs; }
    pure bool operator!=(const Actor &rhs) const { return this != &rhs; }
};

} // namespace nvl

template <>
struct std::hash<nvl::Actor> {
    pure expand U64 operator()(const nvl::Actor &actor) const noexcept;
};
