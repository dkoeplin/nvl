#pragma once

#include "nvl/geo/Tuple.h"
#include "nvl/geo/Volume.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/Pure.h"

namespace nvl::test {

class LabeledBox {
public:
    LabeledBox() = default;
    LabeledBox(const U64 id, const Box<2> &box) : id_(id), box_(box) {}
    pure bool operator==(const LabeledBox &rhs) const { return id_ == rhs.id_ && box_ == rhs.box_; }
    pure bool operator!=(const LabeledBox &rhs) const { return !(*this == rhs); }
    LabeledBox operator+(const Pos<2> &offset) const { return {id_, box_ + offset}; }

    pure U64 id() const { return id_; }
    pure const Box<2> &bbox() const { return box_; }

    void move(const Pos<2> delta) { box_ += delta; }
    void moveto(const Pos<2> pos) { box_ = Box<2>(pos, pos + box_.shape()); }

private:
    U64 id_;
    Box<2> box_;
};

inline std::ostream &operator<<(std::ostream &os, const LabeledBox &v) {
    return os << "BOX(" << v.id() << ": " << v.bbox() << ")";
}

} // namespace nvl::test

template <>
struct std::hash<nvl::test::LabeledBox> {
    pure U64 operator()(const nvl::test::LabeledBox &a) const noexcept { return nvl::sip_hash(a); }
};
