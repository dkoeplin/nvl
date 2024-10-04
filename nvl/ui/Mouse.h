#pragma once

#include "nvl/data/SipHash.h"
#include "nvl/macros/Pure.h"

namespace nvl {

struct Mouse {
    enum Value {
        Left = 0,     // Mouse button left
        Right = 1,    // Mouse button right
        Middle = 2,   // Mouse button middle (pressed wheel)
        Side = 3,     // Mouse button side (advanced mouse device)
        Extra = 4,    // Mouse button extra (advanced mouse device)
        Forward = 5,  // Mouse button forward (advanced mouse device)
        Backward = 6, // Mouse button back (advanced mouse device)
        kNumButtons = 7
    };
    static constexpr Value kButtons[kNumButtons] = {Left, Right, Middle, Side, Extra, Forward, Backward};

    implicit Mouse(const Value value) : value(value) {}
    implicit operator Value() const { return value; }

    Value value;
};

} // namespace nvl

template <>
struct std::hash<nvl::Mouse> {
    pure U64 operator()(const nvl::Mouse mouse) const noexcept { return sip_hash(mouse.value); }
};
