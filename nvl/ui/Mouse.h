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
        kNumButtons = 7,
        Any = 8
    };
    static constexpr Value kButtons[kNumButtons] = {Left, Right, Middle, Side, Extra, Forward, Backward};

    implicit Mouse(const int value) : value(static_cast<Value>(value)) {}
    implicit Mouse(const Value value) : value(value) {}
    implicit operator Value() const { return value; }

    pure std::string to_string() const;

    Value value;
};

inline std::ostream &operator<<(std::ostream &os, const Mouse &mouse) { return os << mouse.to_string(); }

} // namespace nvl

template <>
struct std::hash<nvl::Mouse> {
    pure U64 operator()(const nvl::Mouse mouse) const noexcept { return sip_hash(mouse.value); }
};
