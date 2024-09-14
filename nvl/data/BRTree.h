#pragma once

#include "nvl/data/RTree.h"
#include "nvl/macros/Aliases.h"
#include "nvl/traits/GetBox.h"
#include "nvl/traits/GetID.h"

namespace nvl {

/**
 * @class BRTree
 * @brief Data structure for storing volumes in N-dimensional space with lazily computed borders.
 *
 * @tparam N Number of dimensions in the N-dimensional space.
 * @tparam Value Value type being stored.
 * @tparam kMaxEntries Maximum number of entries per node. Defaults to 10.
 * @tparam kGridExpMin Minimum node grid size (2 ^ min_grid_exp). Defaults to 2.
 * @tparam kGridExpMax Initial grid size of the root. (2 ^ root_grid_exp). Defaults to 10.
 * @tparam GetBox Defines how to fetch the associated volume for a value.
 * @tparam GetID Defines how to fetch the associated ID for a value
 */
template <U64 N, typename Value, U64 kMaxEntries = 10, U64 kGridExpMin = 2, U64 kGridExpMax = 10,
          typename GetBox = traits::GetBox<N, Value>, typename GetID = traits::GetID<Value>>
class BRTree {
public:
    BRTree() = default;

    pure bool has_value_at(const Pos<N> &pos) const { return values_[pos - loc].has_next(); }

    BRTree &insert(const Value &value) {
        values_.insert(value);
        changed_ = true;
        return *this;
    }

    BRTree &remove(const Value &value) {
        values_.remove(value);
        changed_ = true;
        return *this;
    }

    /// Returns the bounding box over all values in this tree.
    pure Pos<N> bbox() const { return values_.bbox(); }

    /// Returns the shape of the bounding box over all values in this tree.
    pure Pos<N> shape() const { return values_.shape(); }

    /// Returns the current number of values in this tree.
    pure U64 size() const { return values_.size(); }

    /// Returns true if this tree is empty.
    pure bool empty() const { return values_.empty(); }

    struct Testing {
        explicit Testing(BRTree &tree) : tree(tree) {}

        /// Returns the number of nodes currently in this tree.
        pure U64 nodes() const { return tree.values_.nodes(); }

        BRTree &tree;
    };
    pure Testing testing() { return Testing(*this); }

    Pos<N> loc = Pos<N>::fill(0);

private:
    friend struct Testing;

    void update() {}

    bool changed_ = false;
    RTree<N, Value, kMaxEntries, kGridExpMin, kGridExpMax, GetBox, GetID> values_;
    RTree<N, Value, kMaxEntries, kGridExpMin, kGridExpMax, GetBox, GetID> borders_;
};

} // namespace nvl
