#pragma once

#include "nvl/data/RTree.h"
#include "nvl/geo/Edge.h"
#include "nvl/geo/View.h"
#include "nvl/macros/Aliases.h"
#include "nvl/traits/HasBox.h"
#include "nvl/traits/HasID.h"

namespace nvl {

/**
 * @class BRTree
 * @brief Data structure for storing volumes in N-dimensional space with lazily computed edges and global offset.
 *
 * @tparam N Number of dimensions in the N-dimensional space.
 * @tparam Value Value type being stored.
 * @tparam kMaxEntries Maximum number of entries per node. Defaults to 10.
 * @tparam kGridExpMin Minimum node grid size (2 ^ min_grid_exp). Defaults to 2.
 * @tparam kGridExpMax Initial grid size of the root. (2 ^ root_grid_exp). Defaults to 10.
 */
template <U64 N, typename Value, U64 kMaxEntries = 10, U64 kGridExpMin = 2, U64 kGridExpMax = 10>
    requires traits::HasBox<Value> && traits::HasID<Value>
class BRTree {
public:
    template <typename Entry>
        requires traits::HasBox<Entry> && traits::HasID<Entry>
    using Tree = RTree<N, Entry, kMaxEntries, kGridExpMin, kGridExpMax>;

    using value_iterator = typename Tree<Value>::value_iterator;

    class view_iterator {
    public:
        using value_type = View<N, Value>;
        using pointer = View<N, Value> *;
        using reference = View<N, Value> &;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::input_iterator_tag;

        static view_iterator begin(const Range<value_iterator> &range, const Pos<N> &offset) {
            return view_iterator(range.begin(), offset);
        }
        static view_iterator end(const Range<value_iterator> &range, const Pos<N> &offset) {
            return view_iterator(range.end(), offset);
        }

        pointer operator->() { return &value_.value(); }
        reference operator*() { return value_.value(); }

        view_iterator &operator++() {
            ++iter_;
            value_ = iter_.has_value() ? Some(View<N, Value>(*iter_, offset_)) : None;
            return *this;
        }

        pure bool operator==(const view_iterator &rhs) { return iter_ == rhs.iter_; }
        pure bool operator!=(const view_iterator &rhs) { return iter_ != rhs.iter_; }

    private:
        explicit view_iterator(value_iterator iter, const Pos<N> &offset) : iter_(iter), offset_(offset) {
            value_ = iter_.has_value() ? Some(View<N, Value>(*iter_, offset_)) : None;
        }
        value_iterator iter_;
        Maybe<View<N, Value>> value_ = None;
        Pos<N> offset_;
    };

    BRTree() = default;

    pure Range<view_iterator> operator[](const Pos<N> &pos) { return Range<view_iterator>(values_[pos - loc], loc); }
    pure Range<view_iterator> operator[](const Box<N> &box) { return Range<view_iterator>(values_[box - loc], loc); }

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
    pure Box<N> bbox() const { return values_.bbox() + loc; }

    /// Returns the shape of the bounding box over all values in this tree.
    pure Pos<N> shape() const { return values_.shape(); }

    /// Returns the current number of values in this tree.
    pure U64 size() const { return values_.size(); }

    /// Returns true if this tree is empty.
    pure bool empty() const { return values_.empty(); }

    struct Debug {
        explicit Debug(BRTree &tree) : tree(tree) {}

        /// Returns the number of nodes currently in this tree.
        pure U64 nodes() const { return tree.values_.debug.nodes(); }

        /// Returns the maximum depth of this tree, in nodes.
        pure U64 depth() const { return tree.values_.debug.depth(); }

        pure const Tree<Value> &rtree() const { return tree.values_; }

        BRTree &tree;
    } debug = Debug(*this);

    Pos<N> loc = Pos<N>::fill(0);

private:
    friend struct Debug;

    void update() {}

    bool changed_ = false;
    Tree<Value> values_;
    Tree<Edge<N>> borders_;
};

} // namespace nvl
