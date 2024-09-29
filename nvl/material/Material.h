#pragma once

#include <memory>
#include <string>

#include "nvl/draw/Color.h"
#include "nvl/macros/Abstract.h"
#include "nvl/macros/Aliases.h"
#include "nvl/reflect/Castable.h"
#include "nvl/reflect/ClassTag.h"

namespace nvl {

struct Material;

abstract struct AbstractMaterial : Castable<Material, AbstractMaterial>::BaseClass {
    class_tag(AbstractMaterial);

    explicit AbstractMaterial(const Color color, const I64 durability, const bool falls = true)
        : color(color), durability(durability), falls(falls) {}

    Color color;
    I64 durability;
    bool falls = true;
};

struct Material final : Castable<Material, AbstractMaterial, std::shared_ptr<AbstractMaterial>> {
    using Castable::Castable;
    using Castable::get;
};

} // namespace nvl
