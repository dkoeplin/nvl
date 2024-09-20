#pragma once

#include "nvl/macros/Aliases.h"
#include "nvl/macros/Expand.h"
#include "nvl/macros/Implicit.h"
#include "nvl/macros/Pure.h"

namespace nvl {

struct Status {
    static const Status Died;
    static const Status Woke;
    static const Status Idle;
    static const Status Move;

    constexpr Status() : value(0) {}
    explicit constexpr Status(const U64 value) : value(value) {}

    pure implicit operator bool() const { return value != 0; }

    pure expand Status operator|(const Status &rhs) const { return Status(value | rhs.value); }
    pure expand Status operator&(const Status &rhs) const { return Status(value & rhs.value); }

    U64 value;
};

constexpr Status Status::Died(1 << 0);
constexpr Status Status::Woke(1 << 1);
constexpr Status Status::Idle(1 << 2);
constexpr Status Status::Move(1 << 3);

} // namespace nvl
