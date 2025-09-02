#pragma once

#include "nvl/macros/Aliases.h"

namespace a2 {

static constexpr I64 kMillisPerTick = 30;
static constexpr I64 kPixelsPerMeter = 1000;
static constexpr I64 kMillisPerSec = 1000;

constexpr I64 operator""_mm(const unsigned long long n) { return n; }

constexpr I64 operator""_cm(const unsigned long long n) { return n * 10LL; }

constexpr I64 operator""_m(const unsigned long long n) { return n * 1'000LL; }

constexpr I64 operator""_km(const unsigned long long n) { return n * 1'000'000LL; }

/// Converts a value in meters/sec to pixels/tick
constexpr I64 operator""_mps(const unsigned long long n) {
    return n * kPixelsPerMeter * kMillisPerTick / kMillisPerSec;
}

/// Converts a value in meters/sec^2 to pixels/tick^2
constexpr I64 operator""_mps2(const unsigned long long n) {
    return n * kPixelsPerMeter * kMillisPerTick * kMillisPerTick / (kMillisPerSec * kMillisPerSec);
}

} // namespace a2
