#pragma once

#include "nvl/macros/Aliases.h"
#include "nvl/macros/Expand.h"
#include "nvl/macros/Implicit.h"
#include "nvl/macros/Pure.h"

namespace nvl {

struct Status {
    static constexpr U64 None = 0;
    static constexpr U64 Died = 1 << 0;
    static constexpr U64 Woke = 1 << 1;
    static constexpr U64 Idle = 1 << 2;
    static constexpr U64 Move = 1 << 3;
    static constexpr U64 Hit = 1 << 4;

    constexpr Status() : value(0) {}
    implicit constexpr Status(const U64 value) : value(value) {}

    pure explicit operator bool() const { return value != 0; }

    pure expand bool operator==(const Status &rhs) const { return value == rhs.value; }
    pure expand bool operator!=(const Status &rhs) const { return value != rhs.value; }

    expand Status &operator|=(const Status &rhs) {
        value |= rhs.value;
        return *this;
    }

    pure expand Status operator|(const Status &rhs) const { return value | rhs.value; }
    pure expand Status operator&(const Status &rhs) const { return value & rhs.value; }

    U64 value;
};

} // namespace nvl
