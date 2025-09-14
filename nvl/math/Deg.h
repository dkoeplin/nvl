#pragma once

#include <cmath>
#include <string>

#include "nvl/macros/Aliases.h"
#include "nvl/macros/Expand.h"
#include "nvl/macros/Implicit.h"
#include "nvl/macros/Pure.h"

namespace nvl {

static constexpr F64 PI = 3.14159265358979323846;
static constexpr F64 kDeg2Rad = PI / 180.0;
static constexpr F64 kRad2Deg = 180.0 / PI;

/**
 * @class Deg
 * @brief Wrapper class around a fixed-point representation for degrees.
 */
class Deg {
public:
    static constexpr int64_t kScaleFactor = 100;
    static constexpr int64_t kDegreeMax = 360 * kScaleFactor;

    struct Lut {
        constexpr Lut() = default;
        pure expand const F64 &operator[](const U64 deg) const { return lut[deg]; }
        pure expand F64 &operator[](const U64 deg) { return lut[deg]; }
        F64 lut[Deg::kDegreeMax] = {};
    };

    /// Returns a Deg directly from its fixed point representation.
    expand static Deg make_raw(const int64_t d) { return Deg(d, true); }

    Deg() : d(0) {}
    expand implicit constexpr Deg(const int degrees) : d(degrees * kScaleFactor) {}
    expand implicit constexpr Deg(const I64 degrees) : d(degrees * kScaleFactor) {}
    expand implicit constexpr Deg(const F64 degrees) : d(static_cast<int64_t>(std::round(degrees * kScaleFactor))) {}
    expand explicit constexpr Deg(const unsigned long long degrees) : d(degrees * kScaleFactor) {}
    expand explicit constexpr Deg(const long double degrees)
        : d(static_cast<int64_t>(std::round(degrees * kScaleFactor))) {}

    expand void operator+=(const Deg rhs) { d += rhs.d; }
    expand void operator-=(const Deg rhs) { d -= rhs.d; }

    pure expand Deg operator-() const { return make_raw(-d); }
    pure expand Deg operator+(const Deg rhs) const { return make_raw(d + rhs.d); }
    pure expand Deg operator-(const Deg rhs) const { return make_raw(d - rhs.d); }

    pure expand std::string to_string() const { return std::to_string(degrees()); }
    pure expand F64 degrees() const { return static_cast<F64>(d) / kScaleFactor; }
    pure expand F64 radians() const { return kDeg2Rad * static_cast<F64>(d) / kScaleFactor; }

    pure expand I64 raw() const { return d; }

private:
    explicit Deg(const int64_t degrees, const bool raw) : d(raw ? degrees : degrees * kScaleFactor) {}
    I64 d;
};

Deg::Lut sin_lut();
Deg::Lut cos_lut();
Deg::Lut tan_lut();

pure expand F64 sin(const Deg deg) {
    static const Deg::Lut lut = sin_lut();
    return lut[(deg.raw() % Deg::kDegreeMax + Deg::kDegreeMax) % Deg::kDegreeMax];
}

pure expand F64 cos(const Deg deg) {
    static const Deg::Lut lut = cos_lut();
    return lut[(deg.raw() % Deg::kDegreeMax + Deg::kDegreeMax) % Deg::kDegreeMax];
}

pure expand F64 tan(const Deg deg) {
    static const Deg::Lut lut = tan_lut();
    return lut[(deg.raw() % Deg::kDegreeMax + Deg::kDegreeMax) % Deg::kDegreeMax];
}

constexpr Deg operator""_deg(const unsigned long long n) { return Deg(n); }
constexpr Deg operator""_deg(const long double n) { return Deg(n); }

expand std::ostream &operator<<(std::ostream &os, const Deg &deg) { return os << deg.to_string(); }

} // namespace nvl
