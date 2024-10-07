#pragma once

#include "nvl/data/SipHash.h"
#include "nvl/macros/Implicit.h"
#include "nvl/macros/Pure.h"

namespace nvl {

struct Scroll {
    enum Value { kHorizontal, kVertical };
    implicit Scroll(const int value) : value(static_cast<Value>(value)) {}
    implicit Scroll(const Value value) : value(value) {}
    implicit operator Value() const { return value; }

    pure std::string to_string() const { return value == kHorizontal ? "X" : "Y"; }

    Value value;
};

inline std::ostream &operator<<(std::ostream &os, const Scroll &scroll) { return os << scroll.to_string(); }

} // namespace nvl

template <>
struct std::hash<nvl::Scroll> {
    pure U64 operator()(const nvl::Scroll scroll) const noexcept { return sip_hash(scroll.value); }
};
