#pragma once

#include <algorithm>

#include "nvl/geo/Dir.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/Implicit.h"
#include "nvl/macros/Pure.h"
#include "nvl/ui/Color.h"

namespace nvl {

struct GlowEffect {
    explicit GlowEffect(const U64 speed = 10, const U64 min = 512, const U64 max = 756)
        : speed(speed), count(min), min(min), max(max) {}
    pure implicit operator Color() const { return {.a = count}; }

    void advance() {
        count += speed * dir;
        if (count > max || count < min) {
            dir = -dir;
            count = std::clamp<U64>(count, min, max);
        }
    }
    U64 speed;
    U64 count;
    Dir dir = Dir::Pos;
    const U64 min;
    const U64 max;
};

} // namespace nvl
