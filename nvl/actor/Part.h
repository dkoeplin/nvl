#pragma once

#include "nvl/draw/Draw.h"
#include "nvl/geo/Box.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/Pure.h"
#include "nvl/material/Material.h"

namespace nvl {

template <U64 N>
class Part {
public:
    explicit Part(const Box<N> &box, Material material, const I64 health)
        : box(box), material(std::move(material)), health(health) {}
    explicit Part(const Box<N> &box, const Material &material) : Part(box, material, material->durability) {}

    pure const Box<N> &bbox() const { return box; }

    pure List<Part> diff(const Box<N> &rhs) const {
        List<Part> result;
        for (const Box<N> &rest : box.diff(rhs)) {
            result.emplace_back(rest, material, health);
        }
        return result;
    }

    void draw(Draw &, const I64) const {
        // TODO: Stub
    }

    Box<N> box;
    Material material;
    I64 health;
};

} // namespace nvl
