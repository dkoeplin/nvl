#pragma once

#include "nvl/data/SipHash.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/Pure.h"
#include "nvl/math/Random.h"

namespace nvl {

struct Color {
    constexpr Color() = default;
    explicit constexpr Color(const U64 r, const U64 g, const U64 b) : Color(r, g, b, 0xFF) {}
    explicit constexpr Color(const U64 r, const U64 g, const U64 b, const U64 a) {
        value = ((a << 24) & 0xFF) | ((r << 16) & 0xFF) | ((g << 8) & 0xFF) | ((b << 0) & 0xFF);
    }

    pure constexpr U64 a() const { return (value >> 24) & 0xFF; }
    pure constexpr U64 r() const { return (value >> 16) & 0xFF; }
    pure constexpr U64 g() const { return (value >> 8) & 0xFF; }
    pure constexpr U64 b() const { return (value >> 0) & 0xFF; }

    pure constexpr bool operator==(const Color &rhs) const { return value == rhs.value; }
    pure constexpr bool operator!=(const Color &rhs) const { return value != rhs.value; }

    U64 value = 0;
};

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
    pure U64 operator()(const nvl::Color &color) const noexcept { return nvl::sip_hash(color.value); }
};
