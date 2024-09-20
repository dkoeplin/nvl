#pragma once

#include "nvl/geo/Box.h"
#include "nvl/macros/Abstract.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/Pure.h"

namespace nvl {

class Draw;
class Message;
class TickResult;

abstract class Actor {
public:
    Actor() = default;
    Actor(const Actor &) = delete;
    Actor &operator=(const Actor &) = delete;
    virtual ~Actor() = default;

    virtual TickResult tick(const List<Message> &messages) = 0;
    virtual void draw(Draw &draw, I64 highlight) const = 0;

    pure bool operator==(const Actor &rhs) const { return this == &rhs; }
    pure bool operator!=(const Actor &rhs) const { return this != &rhs; }
};

} // namespace nvl
