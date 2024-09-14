#pragma once

#include "nvl/geo/Box.h"
#include "nvl/geo/Pos.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/Pure.h"

namespace nvl::testing {

class LabeledBox {
public:
    LabeledBox() = default;
    LabeledBox(const U64 id, const Box<2> &box) : id_(id), box_(box) {}
    pure bool operator==(const LabeledBox &rhs) const { return id_ == rhs.id_ && box_ == rhs.box_; }
    pure bool operator!=(const LabeledBox &rhs) const { return !(*this == rhs); }
    LabeledBox operator+(const Pos<2> &offset) const { return {id_, box_ + offset}; }

    pure U64 id() const { return id_; }
    pure const Box<2> &box() const { return box_; }

private:
    U64 id_;
    Box<2> box_;
};

inline std::ostream &operator<<(std::ostream &os, const LabeledBox &v) {
    return os << "BOX(" << v.id() << ": " << v.box() << ")";
}

} // namespace nvl::testing
