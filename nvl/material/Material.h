#pragma once

#include <string>

#include "nvl/macros/Abstract.h"
#include "nvl/macros/Aliases.h"
#include "nvl/reflect/CastableShared.h"
#include "nvl/reflect/ClassTag.h"
#include "nvl/ui/Color.h"

namespace nvl {

struct Material;

abstract struct AbstractMaterial : CastableShared<Material, AbstractMaterial>::BaseClass {
    class_tag(AbstractMaterial);

    explicit AbstractMaterial(const Color color, const I64 durability, const bool falls = true)
        : color(color), durability(durability), falls(falls) {}

    Color color;
    I64 durability;
    bool falls = true;
    bool outline = true;
};

struct Material final : CastableShared<Material, AbstractMaterial> {
    using CastableShared::CastableShared;
    using CastableShared::get;
};

} // namespace nvl
