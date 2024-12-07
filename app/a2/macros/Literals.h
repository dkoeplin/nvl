#pragma once

#include "nvl/macros/Aliases.h"

namespace a2 {

static constexpr U64 kMillisPerTick = 30;
static constexpr U64 kPixelsPerMeter = 1000;
static constexpr U64 kMillisPerSec = 1000;

constexpr U64 operator"" _mm(const unsigned long long n) { return n; }

constexpr U64 operator"" _cm(const unsigned long long n) { return n * 10; }

constexpr U64 operator"" _m(const unsigned long long n) { return n * 1'000; }

constexpr U64 operator"" _km(const unsigned long long n) { return n * 1'000'000; }

/// Converts a value in meters/sec to pixels/tick
constexpr U64 operator"" _mps(const unsigned long long n) {
    return (n * kPixelsPerMeter * kMillisPerTick) / kMillisPerSec;
}

/// Converts a value in meters/sec^2 to pixels/tick^2
constexpr U64 operator"" _mps2(const unsigned long long n) {
    return (n * kPixelsPerMeter * kMillisPerTick * kMillisPerTick) / (kMillisPerSec * kMillisPerSec);
}

} // namespace a2
