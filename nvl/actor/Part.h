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
        : box_(box), material_(std::move(material)), health_(health) {}
    explicit Part(const Box<N> &box, const Material &material) : Part(box, material, material.durability()) {}

    pure const Box<N> &bbox() const { return box_; }
    pure const Material &material() const { return material_; }
    pure I64 health() const { return health_; }

    pure List<Part> diff(const Box<N> &rhs) const {
        List<Part> result;
        for (const auto &rest : box_.diff(rhs)) {
            result.emplace(rest, material_, health_);
        }
        return result;
    }

    void draw(Draw &, const I64) const {
        // TODO: Stub
    }

private:
    Box<N> box_;
    Material material_;
    I64 health_;
};

} // namespace nvl
