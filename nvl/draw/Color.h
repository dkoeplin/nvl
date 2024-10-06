#pragma once

#include "nvl/data/SipHash.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/Pure.h"
#include "nvl/math/Random.h"

namespace nvl {

struct Color {
    static constexpr U64 kLighter = 1462;
    static constexpr U64 kNormal = 1024;
    static constexpr U64 kDarker = 763;
    struct Options {
        U64 scale = kNormal; // Scales all color channels by (scale/1024)
        U64 alpha = kNormal; // Scales the alpha channel by (alpha/1024)
    };

    static const Color kBlack;
    static const Color kRed;
    static const Color kGreen;
    static const Color kBlue;
    static const Color kWhite;

    // constexpr Color() = default;
    constexpr static Color hex(const U64 hex) {
        return {.r = (hex >> 16) & 0xFF, .g = (hex >> 8) & 0xFF, .b = hex & 0xFF};
    }
    // constexpr Color(const U64 r, const U64 g, const U64 b) : Color(r, g, b, 0xFF) {}
    // constexpr Color(const U64 r, const U64 g, const U64 b, const U64 a) : r(r), g(g), b(b), a(a) {}

    /// Returns a "highlighted" version of this color that scales the R, G, and B channels by (highlight / 1024).
    pure constexpr Color highlight(const Options options) const {
        return {.r = std::min<U64>((r * options.scale) >> 10, 0xFF),
                .g = std::min<U64>((g * options.scale) >> 10, 0xFF),
                .b = std::min<U64>((b * options.scale) >> 10, 0xFF),
                .a = std::min<U64>((a * options.alpha) >> 10, 0xFF)};
    }

    pure constexpr bool operator==(const Color &rhs) const {
        return r == rhs.r && g == rhs.g && b == rhs.b && a == rhs.a;
    }
    pure constexpr bool operator!=(const Color &rhs) const { return !(*this == rhs); }

    U64 r = 0xFF;
    U64 g = 0xFF;
    U64 b = 0xFF;
    U64 a = 0xFF;
};

// clang-format off
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
    /*template <typename I>
    pure Color normal(Random &random, const I mean, const I stddev) const {
        const U64 mn = std::max<I>(0x00, static_cast<U64>(min));
        const U64 std = std::min<I>(static_cast<U64>(max), 0xFF);
        return {.r = random.normal<I>(mean, stddev),
                .g = random.normal<I>(mean, stddev),
                .b = random.normal<I>(mean, stddev)};
    }*/
};

} // namespace nvl

template <>
struct std::hash<nvl::Color> {
    pure U64 operator()(const nvl::Color &color) const noexcept { return nvl::sip_hash(color); }
};
