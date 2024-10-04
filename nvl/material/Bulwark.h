#pragma once

#include "nvl/draw/Color.h"
#include "nvl/material/Material.h"

namespace nvl {

struct Bulwark final : AbstractMaterial {
    class_tag(Bulwark, AbstractMaterial);
    Bulwark() : AbstractMaterial(Color::kBlack, 100, false) {}
};

} // namespace nvl