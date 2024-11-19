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
    explicit View2D(const Pos<2> &offset) : offset(offset) {}

    Pos<2> offset = Pos<2>::zero;
};

struct View3D final : AbstractViewOffset {
    class_tag(View3D, AbstractViewOffset);
    View3D() = default;
    explicit View3D(const Pos<3> &offset) : offset(offset) {}

    void rotate(const Pos<2> &delta, const Pos<2> &shape);

    pure Pos<3> project() const;
    pure Pos<3> project(U64 length) const;
    pure Pos<3> project(const Pos<3> &from, U64 length) const;

    Pos<3> offset = Pos<3>::zero; // Location of camera, in world coordinates
    float pitch = 0;              // Angle between XY, between -89 and 89
    float angle = 0;              // Angle between XZ, between 0 and 360
    float fov = 45;               // Viewing field of view, between 0 and 90
    I64 dist = 1000;              // View distance
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
    template <U64 N>
    static ViewOffset at(const Pos<N> &pt) {
        if constexpr (N == 2) {
            return ViewOffset::get<View2D>(pt);
        } else if constexpr (N == 3) {
            return ViewOffset::get<View3D>(pt);
        } else {
            UNREACHABLE;
        }
    }
};

} // namespace nvl