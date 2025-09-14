#include "nvl/math/Deg.h"

namespace nvl {

Deg::Lut sin_lut() {
    Deg::Lut lut;
    for (I64 d = 0; d < Deg::kDegreeMax; ++d) {
        lut[d] = std::sin(Deg::make_raw(d).radians());
    }
    return lut;
}

Deg::Lut cos_lut() {
    Deg::Lut lut;
    for (I64 d = 0; d < Deg::kDegreeMax; ++d) {
        lut[d] = std::cos(Deg::make_raw(d).radians());
    }
    return lut;
}

Deg::Lut tan_lut() {
    Deg::Lut lut;
    for (I64 d = 0; d < Deg::kDegreeMax; ++d) {
        lut[d] = std::tan(Deg::make_raw(d).radians());
    }
    return lut;
}

} // namespace nvl
