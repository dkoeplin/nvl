#pragma once

#include "nvl/data/Iterator.h"
#include "nvl/geo/At.h"
#include "nvl/geo/Box.h"
#include "nvl/geo/HasBBox.h"
#include "nvl/geo/RTree.h"
#include "nvl/macros/Aliases.h"

namespace nvl {

namespace detail {

template <U64 N, typename Item, typename ItemRef = Ref<Item>, U64 kMaxEntries = 10, U64 kGridExpMin = 2,
          U64 kGridExpMax = 10>
class BRTreeEdges {
protected:
    using ItemTree = RTree<N, Item, ItemRef, kMaxEntries, kGridExpMin, kGridExpMax>;
    using EdgeTree = RTree<N, Edge<N>, Ref<Edge<N>>, kMaxEntries, kGridExpMin, kGridExpMax>;
    static Box<N> bbox(const ItemRef &item) { return static_cast<const Item *>(item.ptr())->bbox(); }

    BRTreeEdges() = default;
    BRTreeEdges(std::initializer_list<Item> items) : items_(items), changed_(true) {}
    BRTreeEdges(std::initializer_list<ItemRef> items) : items_(items), changed_(true) {}
    explicit BRTreeEdges(Range<Item> items) : items_(items), changed_(true) {}
    explicit BRTreeEdges(Range<ItemRef> items) : items_(items), changed_(true) {}

    void mark_changed() { changed_ = true; }

    EdgeTree &get_edges() const {
        if (changed_) {
            // Clear the edges
            changed_ = false;
            edges_.clear();

            // Recompute edges across all values
            for (const ItemRef &item : items_) {
                for (const Edge<N> &edge : bbox(item).edges()) {
                    List<Box<N>> overlap;
                    for (const ItemRef &b : items_[edge.bbox()]) {
                        overlap.push_back(bbox(b));
                    }
                    Range<Box<N>> overlap_range = overlap.range();
                    for (const Edge<N> &remain : edge.diff(overlap_range)) {
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
 * @tparam N Number of dimensions in the N-dimensional space.
 * @tparam Item Value type being stored.
 * @tparam kMaxEntries Maximum number of entries per node. Defaults to 10.
 * @tparam kGridExpMin Minimum node grid size (2 ^ min_grid_exp). Defaults to 2.
 * @tparam kGridExpMax Initial grid size of the root. (2 ^ root_grid_exp). Defaults to 10.
 */
template <U64 N, typename Item, typename ItemRef = Ref<Item>, U64 kMaxEntries = 10, U64 kGridExpMin = 2,
          U64 kGridExpMax = 10>
    requires trait::HasBBox<Item>
class BRTree : detail::BRTreeEdges<N, Item, ItemRef, kMaxEntries, kGridExpMin, kGridExpMax> {
public:
    using Parent = detail::BRTreeEdges<N, Item, ItemRef, kMaxEntries, kGridExpMin, kGridExpMax>;
    using ItemTree = RTree<N, Item, ItemRef, kMaxEntries, kGridExpMin, kGridExpMax>;
    using EdgeTree = RTree<N, Edge<N>, Ref<Edge<N>>, kMaxEntries, kGridExpMin, kGridExpMax>;

    /// Provides an iterator which returns a View of each Item when dereferenced.
    template <typename Entry, typename EntryRef>
    struct view_iterator final : AbstractIteratorCRTP<view_iterator<Entry, EntryRef>, At<N, Entry>> {
        class_tag(view_iterator, AbstractIterator<Entry>);

        template <View Type = View::kImmutable>
        static Iterator<At<N, Entry>, Type> begin(const Range<EntryRef> &range, const Pos<N> &offset) {
            return make_iterator<view_iterator, Type>(range.begin(), offset);
        }
        template <View Type = View::kImmutable>
        static Iterator<At<N, Entry>, Type> end(const Range<EntryRef> &range, const Pos<N> &offset) {
            return make_iterator<view_iterator, Type>(range.end(), offset);
        }

        explicit view_iterator(Iterator<EntryRef> iter, const Pos<N> &offset) : iter_(iter), offset_(offset) {}

        const At<N, Entry> *ptr() override { return &value(); }

        void increment() override {
            ++iter_;
            value_ = None;
        }

        pure bool operator==(const view_iterator &rhs) const override { return iter_ == rhs.iter_; }

    private:
        At<N, Entry> &value() {
            // Views are lazily created to avoid dereferencing an end/empty iterator.
            if (!value_.has_value()) {
                value_ = Some(At<N, Entry>(*iter_, offset_));
            }
            return value_.value();
        }

        Iterator<EntryRef> iter_;
        Maybe<At<N, Entry>> value_ = None;
        Pos<N> offset_;
    };

    BRTree() : Parent() {}
    BRTree(std::initializer_list<Item> items) : Parent(items) {}
    BRTree(std::initializer_list<ItemRef> items) : Parent(items) {}
    explicit BRTree(Range<Item> items) : Parent(items) {}
    explicit BRTree(Range<ItemRef> items) : Parent(items) {}

    BRTree(Pos<2> loc, std::initializer_list<Item> items) : Parent(items), loc(loc) {}
    BRTree(Pos<2> loc, std::initializer_list<ItemRef> items) : Parent(items), loc(loc) {}
    explicit BRTree(Pos<2> loc, Range<Item> items) : Parent(items), loc(loc) {}
    explicit BRTree(Pos<2> loc, Range<ItemRef> items) : Parent(items), loc(loc) {}

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

    /// Returns the bounding box over all values in this tree.
    pure Box<N> bbox() const { return this->items_.bbox() + loc; }

    /// Returns the shape of the bounding box over all values in this tree.
    pure Pos<N> shape() const { return this->items_.shape(); }

    /// Returns the current number of values in this tree.
    pure U64 size() const { return this->items_.size(); }

    /// Returns the number of nodes currently in this tree.
    pure U64 nodes() const { return this->items_.nodes(); }

    /// Returns the maximum depth of this tree, in nodes.
    pure U64 depth() const { return this->items_.depth(); }

    /// Returns true if this tree is empty.
    pure bool empty() const { return this->items_.empty(); }

    pure const ItemTree &item_rtree() const { return this->items_; }
    pure const EdgeTree &edge_rtree() const { return this->get_edges(); }

    using window_iterator = view_iterator<Item, ItemRef>;
    using item_iterator = view_iterator<Item, ItemRef>;
    using edge_iterator = view_iterator<Edge<N>, Ref<Edge<N>>>;

    /// Returns an unordered Range for iteration over all values in this tree in the given volume.
    /// Items are returned as View<N, Item>, where the view is with respect to this tree's global offset.
    pure MRange<At<N, Item>> operator[](const Pos<N> &pos) { return operator[](Box<N>::unit(pos)); }
    pure MRange<At<N, Item>> operator[](const Box<N> &box) {
        return make_mrange<window_iterator>(this->items_[box - loc], loc);
    }
    pure Range<At<N, Item>> operator[](const Pos<N> &pos) const { return operator[](Box<N>::unit(pos)); }
    pure Range<At<N, Item>> operator[](const Box<N> &box) const {
        return make_mrange<window_iterator>(this->items_[box - loc], loc);
    }

    /// Returns an unordered Range for iteration over all values in this tree.
    /// Items are returned as View<N, Item>, where the view is with respect to this tree's global offset.
    pure MRange<At<N, Item>> items() { return make_mrange<item_iterator>(this->items_.items(), loc); }
    pure Range<At<N, Item>> items() const { return make_range<item_iterator>(this->items_.items(), loc); }

    /// Returns true if this item is contained within the tree.
    pure bool has(const ItemRef &item) const { return this->items_.has(item); }

    /// Returns an unordered Range for iteration over all edges in this tree.
    /// Edges are returned as View<N, Edge<N>>, where the view is with respect to this tree's global offset.
    pure Range<At<N, Edge<N>>> edges() const { return make_range<edge_iterator>(this->get_edges().items(), loc); }

    struct Relative {
        using Component = typename UnionFind<ItemRef, typename ItemTree::ItemRefHash>::Group;

        explicit Relative(BRTree &tree) : tree(tree) {}

        /// Returns the connected components in this tree with coordinates relative to this tree's offset.
        pure List<Component> components() { return tree.items_.components(); }

        /// Returns an iterable range over all items in this tree in the given area relative to the tree's offset.
        pure MRange<ItemRef> operator[](const Pos<N> &pos) { return tree.items_[pos]; }
        pure MRange<ItemRef> operator[](const Box<N> &box) { return tree.items_[box]; }
        pure Range<ItemRef> operator[](const Pos<N> &pos) const { return tree.items_[pos]; }
        pure Range<ItemRef> operator[](const Box<N> &box) const { return tree.items_[box]; }

        /// Provides a view to the items contained in this tree relative to the tree's offset.
        pure MRange<ItemRef> items() { return tree.items_.items(); }
        pure Range<ItemRef> items() const { return tree.items_.items(); }

        /// Provides a view to the edges contained in this tree relative to the tree's offset.
        pure Range<Ref<Edge<N>>> edges() const { return tree.get_edges().items(); }

        BRTree &tree;
    } relative = Relative(*this);

    Pos<N> loc = Pos<N>::zero;

private:
    friend struct Relative;
};

} // namespace nvl
