#pragma once

#include <memory>
#include <string>

#include "nvl/draw/Color.h"
#include "nvl/macros/Abstract.h"
#include "nvl/macros/Aliases.h"
#include "nvl/reflect/Castable.h"

namespace nvl {

struct Material;

abstract struct AbstractMaterial : Castable<Material, AbstractMaterial>::BaseClass {
    std::string name;
    Color color;
    bool falls;
    bool liquid;
    I64 durability;
};

struct Material final : Castable<Material, AbstractMaterial> {
    using Castable::Castable;
    using Castable::get;
};

} // namespace nvl
