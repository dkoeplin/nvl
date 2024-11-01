#pragma once

#include "nvl/geo/Box.h"
#include "nvl/geo/Pos.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/Pure.h"

namespace nvl {

template <U64 N>
struct Line {
    pure bool intersects(const Box<N> &box) const;
    Pos<N> a;
    Pos<N> b;
};

template <U64 N>
bool Line<N>::intersects(const Box<N> &box) const {
    return_if(box.contains(a) || box.contains(b), true);
    return_if(!box.overlaps({a, b}), false);
    // Iterate over sides of the box
    for (U64 d = 0; d < N; ++d) {
        for (Dir dir : Dir::list) {}
    }
}

} // namespace nvl
