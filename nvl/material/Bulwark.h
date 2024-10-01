#pragma once

#include "nvl/material/Material.h"

namespace nvl {

struct Bulwark final : AbstractMaterial {
    class_tag(Bulwark, AbstractMaterial);
    Bulwark() : AbstractMaterial(Color(0xFFFFFF), 100, false) {}
};

} // namespace nvl