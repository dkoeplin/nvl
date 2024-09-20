#pragma once

#include "nvl/geo/Box.h"
#include "nvl/geo/HasBBox.h"
#include "nvl/geo/RTree.h"
#include "nvl/geo/View.h"
#include "nvl/macros/Aliases.h"

namespace nvl {

namespace brtree_detail {

template <U64 N, typename Item, U64 kMaxEntries = 10, U64 kGridExpMin = 2, U64 kGridExpMax = 10>
class BRTreeEdges {
protected:
    template <typename Entry>
    using Tree = RTree<N, Entry, kMaxEntries, kGridExpMin, kGridExpMax>;

    void mark_changed() { changed_ = true; }

    Tree<Edge<N>> &get_edges() {
        if (changed_) {
            // Clear the edges
            changed_ = false;
            edges_.clear();

            // Recompute edges across all values
            for (const Item &value : items_.unordered()) {
                for (const Edge<N> &edge : value.bbox().edges()) {
                    for (const Edge<N> &remain : edge.diff(items_[edge.bbox()])) {
                        edges_.insert(remain);
                    }
                }
            }
        }
        return edges_;
    }

    Tree<Item> items_;

private:
    bool changed_ = false;
    Tree<Edge<N>> edges_;
};

} // namespace brtree_detail

/**
 * @class BRTree
 * @brief Data structure for storing volumes in N-dimensional space with lazily computed edges and global offset.
 *
 * @tparam N Number of dimensions in the N-dimensional space.
 * @tparam Item Value type being stored.
 * @tparam kMaxEntries Maximum number of entries per node. Defaults to 10.
 * @tparam kGridExpMin Minimum node grid size (2 ^ min_grid_exp). Defaults to 2.
 * @tparam kGridExpMax Initial grid size of the root. (2 ^ root_grid_exp). Defaults to 10.
 */
template <U64 N, typename Item, U64 kMaxEntries = 10, U64 kGridExpMin = 2, U64 kGridExpMax = 10>
    requires traits::HasBBox<Item>
class BRTree : brtree_detail::BRTreeEdges<N, Item, kMaxEntries, kGridExpMin, kGridExpMax> {
public:
    template <typename Entry>
    using Tree = RTree<N, Entry, kMaxEntries, kGridExpMin, kGridExpMax>;

    /// Provides an iterator which returns a View of each Item when dereferenced.
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

    using window_iterator = view_iterator<typename Tree<Item>::window_iterator, Item>;
    using item_iterator = view_iterator<typename Tree<Item>::item_iterator, Item>;
    using edge_iterator = view_iterator<typename Tree<Edge<N>>::item_iterator, Edge<N>>;

    BRTree() = default;

    pure Range<window_iterator> operator[](const Pos<N> &pos) {
        return Range<window_iterator>(this->items_[pos - loc], loc);
    }
    pure Range<window_iterator> operator[](const Box<N> &box) {
        return Range<window_iterator>(this->items_[box - loc], loc);
    }

    /// Returns an unordered Range for iteration over all values in this tree.
    /// Items are returned as View<N, Item>, where the view is with respect to this tree's global offset.
    pure Range<item_iterator> unordered_items() { return Range<item_iterator>(this->items_.unordered(), loc); }

    /// Returns an unordered Range for iteration over all edges in this tree.
    /// Edges are returned as View<N, Edge<N>>, where the view is with respect to this tree's global offset.
    pure Range<edge_iterator> unordered_edges() { return Range<edge_iterator>(this->get_edges().unordered(), loc); }

    BRTree &insert(const Item &value) {
        this->items_.insert(value);
        this->mark_changed();
        return *this;
    }

    BRTree &remove(const Item &value) {
        this->items_.remove(value);
        this->mark_changed();
        return *this;
    }

    /// Returns the bounding box over all values in this tree.
    pure Box<N> bbox() const { return this->items_.bbox() + loc; }

    /// Returns the shape of the bounding box over all values in this tree.
    pure Pos<N> shape() const { return this->items_.shape(); }

    /// Returns the current number of values in this tree.
    pure U64 size() const { return this->items_.size(); }

    /// Returns true if this tree is empty.
    pure bool empty() const { return this->items_.empty(); }

    struct Debug {
        explicit Debug(BRTree &tree) : tree(tree) {}

        /// Returns the number of nodes currently in this tree.
        pure U64 nodes() const { return tree.items_.debug.nodes(); }

        /// Returns the maximum depth of this tree, in nodes.
        pure U64 depth() const { return tree.items_.debug.depth(); }

        pure const Tree<Item> &item_rtree() const { return tree.items_; }
        pure const Tree<Edge<N>> &edge_rtree() const { return tree.get_edges(); }

        BRTree &tree;
    } debug = Debug(*this);

    struct Viewed {
        using window_iterator = typename Tree<Item>::window_iterator;
        using item_iterator = typename Tree<Item>::item_iterator;
        using edge_iterator = typename Tree<Edge<N>>::item_iterator;

        explicit Viewed(BRTree &tree) : tree(tree) {}

        pure Range<window_iterator> operator[](const Pos<N> &pos) { return tree.items_[pos]; }
        pure Range<window_iterator> operator[](const Box<N> &box) { return tree.items_[box]; }

        pure Range<item_iterator> unordered_items() const { return tree.items_.unordered(); }

        pure Range<edge_iterator> unordered_edges() const { return tree.get_edges().unordered(); }

        BRTree &tree;
    } view = Viewed(*this);

    Pos<N> loc = Pos<N>::fill(0);

private:
    friend struct Debug;
    friend struct Viewed;
};

} // namespace nvl
