#pragma once

#include "nvl/data/SipHash.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/Pure.h"

namespace nvl {

struct Color {
    explicit Color(const U64 r, const U64 g, const U64 b) : Color(r, g, b, 0xFF) {}
    explicit Color(const U64 r, const U64 g, const U64 b, const U64 a) {
        value = ((a << 24) & 0xFF) | ((r << 16) & 0xFF) | ((g << 8) & 0xFF) | ((b << 0) & 0xFF);
    }

    pure U64 a() const { return (value >> 24) & 0xFF; }
    pure U64 r() const { return (value >> 16) & 0xFF; }
    pure U64 g() const { return (value >> 8) & 0xFF; }
    pure U64 b() const { return (value >> 0) & 0xFF; }

    pure bool operator==(const Color &rhs) const { return value == rhs.value; }
    pure bool operator!=(const Color &rhs) const { return value != rhs.value; }

    U64 value;
};

} // namespace nvl

template <>
struct std::hash<nvl::Color> {
    pure U64 operator()(const nvl::Color &color) const noexcept { return nvl::sip_hash(color.value); }
};
