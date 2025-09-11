#include "nvl/math/FastTrig.h"

#include <cmath>

#include "nvl/macros/Expand.h"
#include "nvl/macros/Pure.h"
#include "nvl/math/Trig.h"

namespace nvl::fasttrig {

namespace {

struct DegLut {
    constexpr DegLut() = default;
    pure expand const F64 &operator[](const U64 deg) const { return lut[deg]; }
    pure expand F64 &operator[](const U64 deg) { return lut[deg]; }
    F64 lut[360] = {};
};

DegLut sin_lut() {
    DegLut lut;
    for (U64 d = 0; d < 360; ++d) {
        lut[d] = std::sin(kDeg2Rad * static_cast<float>(d));
    }
    return lut;
}

DegLut cos_lut() {
    DegLut lut;
    for (U64 d = 0; d < 360; ++d) {
        lut[d] = std::cos(kDeg2Rad * static_cast<float>(d));
    }
    return lut;
}

DegLut tan_lut() {
    DegLut lut;
    for (U64 d = 0; d < 360; ++d) {
        lut[d] = std::tan(kDeg2Rad * static_cast<float>(d));
    }
    return lut;
}

} // namespace

pure F64 sin(const Deg deg) {
    static const DegLut lut = sin_lut();
    return lut[(deg % 360 + 360) % 360];
}

pure F64 cos(const Deg deg) {
    static const DegLut lut = cos_lut();
    return lut[(deg % 360 + 360) % 360];
}

pure F64 tan(const Deg deg) {
    static const DegLut lut = tan_lut();
    return lut[(deg % 360 + 360) % 360];
}

pure Deg atan(I64 x, I64 y) { return Deg(std::round(std::atan(y / x) * kRad2Deg) + ((x < 0) ? 180 : 0)); }

} // namespace nvl::fasttrig
