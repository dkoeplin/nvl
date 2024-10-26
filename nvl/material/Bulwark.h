#pragma once

#include "nvl/material/Material.h"
#include "nvl/ui/Color.h"

namespace nvl {

struct Bulwark final : AbstractMaterial {
    class_tag(Bulwark, AbstractMaterial);
    explicit Bulwark(const Color &color = Color::kBlack) : AbstractMaterial(color, 10000, false) {}
};

} // namespace nvl