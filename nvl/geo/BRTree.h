#pragma once

#include "nvl/geo/RTree.h"
#include "nvl/geo/Box.h"
#include "nvl/geo/View.h"
#include "nvl/macros/Aliases.h"
#include "nvl/traits/HasBox.h"
#include "nvl/traits/HasID.h"

namespace nvl {

namespace brtree_detail {

template <U64 N, typename Value, U64 kMaxEntries = 10, U64 kGridExpMin = 2, U64 kGridExpMax = 10>
class BRTreeBorders {
protected:
    template <typename Entry>
    using Tree = RTree<N, Entry, kMaxEntries, kGridExpMin, kGridExpMax>;

    void mark_changed() { changed_ = true; }

    Tree<Edge<N>> &get_borders() {
        if (changed_) {
            // Clear the borders
            changed_ = false;
            borders_.clear();

            // Recompute borders across all values
            for (const Value &value : values_.unordered()) {
                for (const Edge<N> &border : value.box().borders()) {
                    for (const Edge<N> &remain : border.diff(values_[border.box()])) {
                        borders_.insert(remain);
                    }
                }
            }
        }
        return borders_;
    }

    Tree<Value> values_;

private:
    bool changed_ = false;
    Tree<Edge<N>> borders_;
};

} // namespace brtree_detail

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
class BRTree : brtree_detail::BRTreeBorders<N, Value, kMaxEntries, kGridExpMin, kGridExpMax> {
public:
    template <typename Entry>
    using Tree = RTree<N, Entry, kMaxEntries, kGridExpMin, kGridExpMax>;

    /// Provides an iterator which returns a View of each Value when dereferenced.
    template <typename iterator, typename Entry>
    class view_iterator {
    public:
        using value_type = View<N, Entry>;
        using pointer = View<N, Entry> *;
        using reference = View<N, Entry> &;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::input_iterator_tag;

        static view_iterator begin(const Range<iterator> &range, const Pos<N> &offset) {
            return view_iterator(range.begin(), offset);
        }
        static view_iterator end(const Range<iterator> &range, const Pos<N> &offset) {
            return view_iterator(range.end(), offset);
        }

        pointer operator->() { return &value(); }
        reference operator*() { return value(); }

        view_iterator &operator++() {
            ++iter_;
            value_ = None;
            return *this;
        }

        pure bool operator==(const view_iterator &rhs) const { return iter_ == rhs.iter_; }
        pure bool operator!=(const view_iterator &rhs) const { return iter_ != rhs.iter_; }

    private:
        explicit view_iterator(iterator iter, const Pos<N> &offset) : iter_(iter), offset_(offset) {}

        View<N, Entry> &value() {
            // Views are lazily created to avoid dereferencing an end/empty iterator.
            if (!value_.has_value()) {
                value_ = Some(View<N, Entry>(*iter_, offset_));
            }
            return value_.value();
        }

        iterator iter_;
        Maybe<View<N, Entry>> value_ = None;
        Pos<N> offset_;
    };

    using window_iterator = view_iterator<typename Tree<Value>::window_iterator, Value>;
    using value_iterator = view_iterator<typename Tree<Value>::value_iterator, Value>;
    using border_iterator = view_iterator<typename Tree<Edge<N>>::value_iterator, Edge<N>>;

    BRTree() = default;

    pure Range<window_iterator> operator[](const Pos<N> &pos) {
        return Range<window_iterator>(this->values_[pos - loc], loc);
    }
    pure Range<window_iterator> operator[](const Box<N> &box) {
        return Range<window_iterator>(this->values_[box - loc], loc);
    }

    /// Returns an unordered Range for iteration over all values in this tree.
    /// Values are returned as View<N, Value>, where the view is with respect to this tree's global offset.
    pure Range<value_iterator> unordered() const { return Range<value_iterator>(this->values_.unordered(), loc); }

    /// Returns an unordered Range for iteration over all borders in this tree.
    /// Borders are returned as View<N, Edge<N>>, where the view is with respect to this tree's global offset.
    pure Range<border_iterator> borders() { return Range<border_iterator>(this->get_borders().unordered(), loc); }

    BRTree &insert(const Value &value) {
        this->values_.insert(value);
        this->mark_changed();
        return *this;
    }

    BRTree &remove(const Value &value) {
        this->values_.remove(value);
        this->mark_changed();
        return *this;
    }

    /// Returns the bounding box over all values in this tree.
    pure Box<N> bbox() const { return this->values_.bbox() + loc; }

    /// Returns the shape of the bounding box over all values in this tree.
    pure Pos<N> shape() const { return this->values_.shape(); }

    /// Returns the current number of values in this tree.
    pure U64 size() const { return this->values_.size(); }

    /// Returns true if this tree is empty.
    pure bool empty() const { return this->values_.empty(); }

    struct Debug {
        explicit Debug(BRTree &tree) : tree(tree) {}

        /// Returns the number of nodes currently in this tree.
        pure U64 nodes() const { return tree.values_.debug.nodes(); }

        /// Returns the maximum depth of this tree, in nodes.
        pure U64 depth() const { return tree.values_.debug.depth(); }

        pure const Tree<Value> &rtree() const { return tree.values_; }

        BRTree &tree;
    } debug = Debug(*this);

    struct Viewed {
        using window_iterator = typename Tree<Value>::window_iterator;
        using value_iterator = typename Tree<Value>::value_iterator;
        using border_iterator = typename Tree<Edge<N>>::value_iterator;

        explicit Viewed(BRTree &tree) : tree(tree) {}

        pure Range<window_iterator> operator[](const Pos<N> &pos) { return tree.values_[pos]; }
        pure Range<window_iterator> operator[](const Box<N> &box) { return tree.values_[box]; }

        pure Range<value_iterator> unordered() const { return tree.values_.unordered(); }

        pure Range<border_iterator> borders() const { return tree.get_borders().unordered(); }

        BRTree &tree;
    } view = Viewed(*this);

    Pos<N> loc = Pos<N>::fill(0);

private:
    friend struct Debug;
    friend struct Viewed;
};

} // namespace nvl
