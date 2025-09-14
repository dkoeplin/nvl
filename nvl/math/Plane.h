#pragma once

#include "nvl/data/List.h"
#include "nvl/macros/Aliases.h"

namespace nvl {

class Plane {
public:
    template <U64 N>
    static const List<Plane> &all() {
        static const List<Plane> list = make_list<N>();
        return list;
    }

    U64 axis0;
    U64 axis1;
    U64 index;

private:
    explicit constexpr Plane(const U64 axis0, const U64 axis1, const U64 index)
        : axis0(axis0), axis1(axis1), index(index) {}

    template <U64 N>
    static List<Plane> make_list() {
        List<Plane> list;
        U64 index = 0;
        for (U64 a0 = 0; a0 < N; ++a0) {
            for (U64 a1 = a0 + 1; a1 < N; ++a1) {
                list.push_back(Plane(a0, a1, index));
                ++index;
            }
        }
        return list;
    }
};

static const Plane kPlane2D = Plane::all<2>()[0];

namespace Axis3D {
static const Plane &kXY = Plane::all<3>()[0];
static const Plane &kXZ = Plane::all<3>()[1];
static const Plane &kYZ = Plane::all<3>()[2];
} // namespace Axis3D

} // namespace nvl
