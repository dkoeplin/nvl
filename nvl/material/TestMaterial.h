#pragma once

#include "nvl/draw/Color.h"
#include "nvl/material/Material.h"
#include "nvl/reflect/ClassTag.h"

namespace nvl {

struct TestMaterial final : AbstractMaterial {
    class_tag(TestMaterial, AbstractMaterial);
    explicit TestMaterial(const Color color) : AbstractMaterial(color, 2) {}
};

} // namespace nvl
