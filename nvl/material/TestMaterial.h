#pragma once

#include "nvl/material/Material.h"
#include "nvl/reflect/ClassTag.h"
#include "nvl/ui/Color.h"

namespace nvl {

struct TestMaterial final : AbstractMaterial {
    class_tag(TestMaterial, AbstractMaterial);
    explicit TestMaterial(const Color &color) : AbstractMaterial(color, 1) {}
};

} // namespace nvl
