#pragma once

#include "nvl/enum/Dir.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/Pure.h"

namespace nvl {

template <U64 N>
class Box;

template <U64 N>
class Edge {
public:
    explicit Edge(const U64 dim, const Dir dir, const Box<N> &box) : dim_(dim), dir_(dir), box_(box) {}

    pure U64 thickness() const { return box_.shape(dim_); }

    pure U64 id() const { return static_cast<U64>(this); }
    pure const Box<N> &box() const { return box_; }

private:
    U64 dim_;
    Dir dir_;
    Box<N> box_;
};

} // namespace nvl