#pragma once

#include "nvl/geo/Box.h"
#include "nvl/geo/Pos.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/Pure.h"

namespace nvl::testing {

struct LabeledBox {
    LabeledBox() = default;
    LabeledBox(const U64 id, const Box<2> &box) : id(id), box(box) {}
    pure bool operator==(const LabeledBox &rhs) const { return id == rhs.id && box == rhs.box; }
    pure bool operator!=(const LabeledBox &rhs) const { return !(*this == rhs); }
    LabeledBox operator+(const Pos<2> &offset) const { return {id, box + offset}; }
    U64 id;
    Box<2> box;
};

inline std::ostream &operator<<(std::ostream &os, const LabeledBox &v) { return os << "BOX(" << v.id << ": " << v.box << ")"; }

} // namespace nvl::testing
