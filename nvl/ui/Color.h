#pragma once

#include "nvl/data/SipHash.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/Pure.h"
#include "nvl/math/Random.h"

namespace nvl {

struct Color {
    static const Color kLighter;
    static const Color kNormal;
    static const Color kDarker;
    static const Color kMoreTransparent;
    static const Color kLessTransparent;

    static const Color kBlack;
    static const Color kRed;
    static const Color kGreen;
    static const Color kBlue;
    static const Color kWhite;

    constexpr static Color hex(const U64 hex) {
        return {.r = (hex >> 16) & 0xFF, .g = (hex >> 8) & 0xFF, .b = hex & 0xFF};
    }

    /// Returns a scaled version of this color that multiplies the R, G, and B channels by (highlight / 1024).
    pure constexpr Color highlight(const Color &highlight) const {
        return {.r = std::min<U64>((r * highlight.r) / 1024, 0xFF),
                .g = std::min<U64>((g * highlight.g) / 1024, 0xFF),
                .b = std::min<U64>((b * highlight.b) / 1024, 0xFF),
                .a = std::min<U64>((a * highlight.a) / 1024, 0xFF)};
    }

    pure constexpr bool operator==(const Color &rhs) const {
        return r == rhs.r && g == rhs.g && b == rhs.b && a == rhs.a;
    }
    pure constexpr bool operator!=(const Color &rhs) const { return !(*this == rhs); }

    pure std::string to_string() const;

    U64 r = 0xFF;
    U64 g = 0xFF;
    U64 b = 0xFF;
    U64 a = 0xFF;
};

// clang-format off
constexpr Color Color::kLighter {.r = 1462, .g = 1462, .b = 1462, .a = 1024};
constexpr Color Color::kNormal {.r = 1024, .g = 1024, .b = 1024, .a = 1024};
constexpr Color Color::kDarker {.r = 763, .g = 763, .b = 763, .a = 1024};
constexpr Color Color::kMoreTransparent {.r = 1024, .g = 1024, .b = 1024, .a = 763};
constexpr Color Color::kLessTransparent {.r = 1024, .g = 1024, .b = 1024, .a = 1462};

constexpr Color Color::kBlack = hex(0x000000);
constexpr Color Color::kRed   = hex(0xFF0000);
constexpr Color Color::kGreen = hex(0x00FF00);
constexpr Color Color::kBlue  = hex(0x0000FF);
constexpr Color Color::kWhite = hex(0xFFFFFF);
// clang-format on

template <>
struct nvl::RandomGen<Color> {
    template <typename I>
    pure Color uniform(Random &random, const I min, const I max) const {
        const U64 mn = std::max<I>(0x00, static_cast<U64>(min));
        const U64 mx = std::min<I>(static_cast<U64>(max), 0xFF);
        return {.r = random.uniform<U64>(mn, mx), .g = random.uniform<U64>(mn, mx), .b = random.uniform<U64>(mn, mx)};
    }
};

inline std::ostream &operator<<(std::ostream &os, const Color &color) { return os << color.to_string(); }

} // namespace nvl

template <>
struct std::hash<nvl::Color> {
    pure U64 operator()(const nvl::Color &color) const noexcept { return nvl::sip_hash(color); }
};
