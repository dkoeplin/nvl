#pragma once

#include "nox/enum/Dir.h"
#include "nox/macros/Aliases.h"
#include "nox/macros/Pure.h"

namespace nox {

template <U64 N> class Box;

template <U64 N> class Edge {
  public:
    explicit Edge(U64 dim, Dir dir, const Box<N> &box) : dim(dim), dir(dir), box(box) {}

    pure U64 thickness() const { return box.shape(dim); }

    U64 dim;
    Dir dir;
    const Box<N> &box;
};

} // namespace nox