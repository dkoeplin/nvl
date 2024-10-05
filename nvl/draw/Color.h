#pragma once

#include "nvl/data/SipHash.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/Pure.h"
#include "nvl/math/Random.h"
#include "raylib.h"

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

    constexpr Color() = default;
    constexpr static Color Color24b(const U64 hex) { return Color((hex >> 16) & 0xFF, (hex >> 8) & 0xFF, hex & 0xFF); }
    explicit constexpr Color(const U64 r, const U64 g, const U64 b) : Color(r, g, b, 0xFF) {}
    explicit constexpr Color(const U64 r, const U64 g, const U64 b, const U64 a) : color32(r, g, b, a) {}

    implicit Color(const ::Color color) : color32(color) {}
    implicit operator const ::Color() const { return color32; }

    pure constexpr U64 r() const { return color32.r; }
    pure constexpr U64 g() const { return color32.g; }
    pure constexpr U64 b() const { return color32.b; }
    pure constexpr U64 a() const { return color32.a; }

    /// Returns a "highlighted" version of this color that scales the R, G, and B channels by (highlight / 1024).
    pure constexpr Color highlight(const Options options) const {
        const U64 r = std::min((this->r() * options.scale) >> 10, U64(0xFF));
        const U64 g = std::min((this->g() * options.scale) >> 10, U64(0xFF));
        const U64 b = std::min((this->b() * options.scale) >> 10, U64(0xFF));
        const U64 a = std::min((this->a() * options.alpha) >> 10, U64(0xFF));
        return Color(r, g, b, a);
    }

    pure constexpr bool operator==(const Color &rhs) const {
        return color32.r == rhs.color32.r && color32.g == rhs.color32.g && color32.b == rhs.color32.b &&
               color32.a == rhs.color32.a;
    }
    pure constexpr bool operator!=(const Color &rhs) const { return !(*this == rhs); }

    ::Color color32;
};

constexpr Color Color::kBlack = Color(0, 0, 0);
constexpr Color Color::kRed = Color(0xFF, 0, 0);
constexpr Color Color::kGreen = Color(0, 0xFF, 0);
constexpr Color Color::kBlue = Color(0, 0, 0xFF);
constexpr Color Color::kWhite = Color(0xFF, 0xFF, 0xFF);

template <>
struct nvl::RandomGen<Color> {
    template <typename I>
    pure Color uniform(Random &random, const I min, const I max) const {
        const auto r = random.uniform<I>(min, max);
        const auto g = random.uniform<I>(min, max);
        const auto b = random.uniform<I>(min, max);
        return Color(r, g, b);
    }
    template <typename I>
    pure Color normal(Random &random, const I mean, const I stddev) const {
        const auto r = random.normal<I>(mean, stddev);
        const auto g = random.normal<I>(mean, stddev);
        const auto b = random.normal<I>(mean, stddev);
        return Color(r, g, b);
    }
};

} // namespace nvl

template <>
struct std::hash<nvl::Color> {
    pure U64 operator()(const nvl::Color &color) const noexcept { return nvl::sip_hash(color.color32); }
};
