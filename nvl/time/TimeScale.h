#pragma once

#include <string>

#include "nvl/macros/Implicit.h"
#include "nvl/macros/Pure.h"

namespace nvl {

struct TimeScale {
    enum Value { kNanoseconds, kMicroseconds, kMilliseconds, kSeconds, kMinutes, kHours, kDays, kNScales };

    /// All available time scales. Defined from smallest to largest.
    static constexpr Value scales[kNScales]{kNanoseconds, kMicroseconds, kMilliseconds, kSeconds,
                                            kMinutes,     kHours,        kDays};

    /// All units for time scales.
    static constexpr std::string_view unit[kNScales]{"ns", "us", "ms", "sec", "min", "hr", "days"};
    static constexpr U64 divisors[kNScales]{1, 1000, 1000, 1000, 60, 60, 24};

    constexpr TimeScale() = default;
    implicit constexpr TimeScale(const Value value) : value(value) {}
    implicit operator Value() const { return value; }

    pure expand std::string_view units() const { return value < 0 || value >= kNScales ? "unknown" : unit[value]; }

    Value value = kHours;
};

inline std::ostream &operator<<(std::ostream &os, const TimeScale scale) { return os << scale.units(); }

} // namespace nvl