#pragma once

#include "nvl/geo/HasBBox.h"
#include "nvl/geo/Line.h"
#include "nvl/geo/Rel.h"
#include "nvl/geo/RTree.h"
#include "nvl/geo/Volume.h"
#include "nvl/macros/Aliases.h"

namespace nvl {

namespace detail {

template <U64 N, typename Item, typename ItemRef = Rel<Item>, U64 kMaxEntries = 10, U64 kGridExpMin = 2>
    requires trait::HasBBox<N, I64, Item>
class BRTreeEdges {
protected:
    using Edge = nvl::Edge<N, I64>;
    using EdgeRef = Rel<Edge>;
    using ItemTree = RTree<N, Item, ItemRef, kMaxEntries, kGridExpMin>;
    using EdgeTree = RTree<N, Edge, EdgeRef, kMaxEntries, kGridExpMin>;
    static Box<N> bbox(const ItemRef &item) { return static_cast<const Item *>(item.ptr())->bbox(); }

    BRTreeEdges() = default;
    BRTreeEdges(std::initializer_list<Item> items) : items_(items), changed_(true) {}
    BRTreeEdges(std::initializer_list<ItemRef> items) : items_(items), changed_(true) {}
    explicit BRTreeEdges(Range<Item> items) : items_(items), changed_(true) {}
    explicit BRTreeEdges(Range<ItemRef> items) : items_(items), changed_(true) {}

    void mark_changed() const { changed_ = true; }

    EdgeTree &get_edges() const {
        if (changed_) {
            // Clear the edges
            changed_ = false;
            edges_.clear();

            // Recompute edges across all values
            for (const ItemRef &item : items_.items()) {
                for (const auto &edge : bbox(item).edges()) {
                    List<Box<N>> overlap;
                    for (const ItemRef &b : items_[edge.bbox()]) {
                        overlap.push_back(bbox(b));
                    }
                    Range<Box<N>> overlap_range = overlap.range();
                    for (const Edge &remain : edge.diff(overlap_range)) {
                        edges_.insert(remain);
                    }
                }
            }
        }
        return edges_;
    }

    ItemTree items_;

private:
    mutable bool changed_ = false;
    mutable EdgeTree edges_;
};

} // namespace detail

/**
 * @class BRTree
 * @brief Data structure for storing volumes in N-dimensional space with lazily computed edges and global offset.
 *
 * @tparam N - Number of dimensions in the N-dimensional space.
 * @tparam Item - Value type being stored.
 * @tparam ItemRef - Type to use for providing references to items held in this tree. Default is Rel<Item>.
 * @tparam kMaxEntries - Maximum number of entries per node. Defaults to 10.
 * @tparam kGridExpMin - Minimum node grid size (2 ** min_grid_exp). Defaults to 2.
 */
template <U64 N, typename Item, typename ItemRef = Rel<Item>, U64 kMaxEntries = 10, U64 kGridExpMin = 2>
    requires trait::HasBBox<N, I64, Item>
class BRTree : detail::BRTreeEdges<N, Item, ItemRef, kMaxEntries, kGridExpMin> {
public:
    using Edge = nvl::Edge<N, I64>;
    using EdgeRef = Rel<Edge>;
    using Parent = detail::BRTreeEdges<N, Item, ItemRef, kMaxEntries, kGridExpMin>;
    using ItemTree = RTree<N, Item, ItemRef, kMaxEntries, kGridExpMin>;
    using EdgeTree = RTree<N, Edge, EdgeRef, kMaxEntries, kGridExpMin>;
    using Intersect = ItemTree::Intersect;

    BRTree() : Parent() {}
    BRTree(std::initializer_list<Item> items) : Parent(items) {}
    BRTree(std::initializer_list<ItemRef> items) : Parent(items) {}
    explicit BRTree(Range<Item> items) : Parent(items) {}
    explicit BRTree(Range<ItemRef> items) : Parent(items) {}

    BRTree(Pos<N> loc, std::initializer_list<Item> items) : Parent(items), loc(loc) {}
    BRTree(Pos<N> loc, std::initializer_list<ItemRef> items) : Parent(items), loc(loc) {}
    explicit BRTree(Pos<N> loc, Range<Item> items) : Parent(items), loc(loc) {}
    explicit BRTree(Pos<N> loc, Range<ItemRef> items) : Parent(items), loc(loc) {}

    ItemRef insert(const Item &item) {
        auto ref = this->items_.insert(item);
        this->mark_changed();
        return ref;
    }

    BRTree &insert(const Range<Item> &items) {
        this->items_.insert(items);
        this->mark_changed();
        return *this;
    }

    template <typename T = Item, typename... Args>
    ItemRef emplace(Args &&...args) {
        auto ref = this->items_.template emplace<T>(std::forward<Args>(args)...);
        this->mark_changed();
        return ref;
    }

    BRTree &remove(const ItemRef item) {
        this->items_.remove(item);
        this->mark_changed();
        return *this;
    }

    /// Returns a set of all stored items in the given volume.
    pure expand Set<ItemRef> operator[](const Box<N> &box) const { return this->items_[box - loc]; }
    pure expand Set<ItemRef> operator[](const Pos<N> &pos) const { return this->items_[pos - loc]; }

    /// Returns the first item stored in the given volume, if one exists.
    pure expand Maybe<ItemRef> first(const Box<N> &box) const { return this->items_.first(box - loc); }
    pure expand Maybe<ItemRef> first(const Pos<N> &pos) const { return this->items_.first(pos - loc); }
    pure expand Maybe<Intersect> first(const Line<N> &line) const { return this->items_.first(line - loc); }

    /// Returns the closest item which intersects with the line segment according to the distance function.
    /// Also returns the location and face of the intersection if it exists.
    template <typename DistanceFunc> // Intersect => Maybe<F64>
    pure expand Maybe<Intersect> first_where(const Line<N> &line, DistanceFunc dist) const {
        return this->items_.first_where(line - loc, dist);
    }

    /// Returns true if there are any items stored in the given volume.
    pure expand bool exists(const Box<N> &box) const { return this->items_.exists(box - loc); }
    pure expand bool exists(const Pos<N> &pos) const { return this->items_.exists(pos - loc); }

    pure const ItemTree &item_rtree() const { return this->items_; }
    pure const EdgeTree &edge_rtree() const { return this->get_edges(); }

    pure Range<ItemRef> items() const { return this->items_.items(); }
    pure Range<EdgeRef> edges() const { return edge_rtree().items(); }

    pure List<Set<ItemRef>> components() const { return this->items_.components(); }

    /// Returns the bounding box over all values in this tree.
    pure Box<N> bbox() const { return this->items_.bbox() + loc; }

    /// Returns the shape of the bounding box for this tree.
    pure Pos<N> shape() const { return this->items_.shape(); }

    /// Returns the total number of distinct items stored in this tree.
    pure U64 size() const { return this->items_.size(); }

    /// Returns the total number of nodes in this tree.
    pure U64 nodes() const { return this->items_.nodes(); }

    /// Returns true if this tree is empty.
    pure bool empty() const { return this->items_.empty(); }

    /// Returns the maximum depth, in nodes, of this tree.
    pure U64 depth() const { return this->items_.depth(); }

    Pos<N> loc = Pos<N>::zero;
};

} // namespace nvl
