#pragma once

#include "nvl/geo/Pos.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/Unreachable.h"
#include "nvl/reflect/Castable.h"

namespace nvl {
struct ViewOffset;

struct AbstractViewOffset : Castable<ViewOffset, AbstractViewOffset, std::shared_ptr<AbstractViewOffset>>::BaseClass {
    class_tag(AbstractViewOffset);
};

struct View2D final : AbstractViewOffset {
    class_tag(View2D, AbstractViewOffset);
    View2D() = default;
    Pos<2> offset = Pos<2>::zero;
};

struct View3D final : AbstractViewOffset {
    class_tag(View3D, AbstractViewOffset);
    View3D() = default;

    void rotate(const Pos<2> &delta, const Pos<2> &shape);

    pure Pos<3> project(U64 length) const;
    pure Pos<3> project(const Pos<3> &from, U64 length) const;

    Pos<3> offset = Pos<3>::zero;
    float pitch = 0; // Between -90 and 90
    float angle = 0; // Between -180 and 180
    I64 fov = 45;    // Between 0 and 90
};

struct ViewOffset final : Castable<ViewOffset, AbstractViewOffset, std::shared_ptr<AbstractViewOffset>> {
    using Castable::Castable;
    template <U64 N>
    static ViewOffset zero() {
        if constexpr (N == 2) {
            return ViewOffset::get<View2D>();
        } else if constexpr (N == 3) {
            return ViewOffset::get<View3D>();
        } else {
            UNREACHABLE;
        }
    }
};

} // namespace nvl