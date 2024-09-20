#pragma once

#include "nvl/geo/Box.h"
#include "nvl/geo/BRTree.h"

namespace nvl {

template <U64 N>
class Entity;

template <U64 N>
class World {
public:
    static constexpr U64 kMaxEntries = 10;
    static constexpr U64 kGridExpMin = 2;
    static constexpr U64 kGridExpMax = 10;
    using EntityTree = BRTree<N, Entity<N>, kMaxEntries, kGridExpMin, kGridExpMax>;

    pure typename EntityTree::window_iterator entities(const Pos<N> &pos) { return entities_[pos]; }
    pure typename EntityTree::window_iterator entities(const Box<N> &box) { return entities_[box]; }

private:
    EntityTree entities_;
};

} // namespace nvl
