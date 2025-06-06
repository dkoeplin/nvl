#pragma once

#include <memory>

#include "nvl/data/List.h"
#include "nvl/data/Map.h"
#include "nvl/data/Once.h"
#include "nvl/data/Range.h"
#include "nvl/data/Ref.h"
#include "nvl/data/Set.h"
#include "nvl/data/UnionFind.h"
#include "nvl/data/WalkResult.h"
#include "nvl/geo/HasBBox.h"
#include "nvl/geo/Line.h"
#include "nvl/geo/Tuple.h"
#include "nvl/geo/Volume.h"
#include "nvl/io/IO.h"
#include "nvl/macros/Abstract.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/Hot.h"
#include "nvl/macros/ReturnIf.h"
#include "nvl/math/Bitwise.h"

namespace nvl {

namespace detail {

/**
 * @struct Orthants
 * @brief "Orthants" is the ND version of 2D quadrants!
 * @tparam N - Number of dimensions
 */
template <U64 N>
struct Orthants {
    explicit Orthants(const Pos<N> &origin, const U64 size) : origin(origin), size(size) {}

    pure Box<N> box() const { return {origin - size, origin + size}; }

    /// Return a flattened index which addresses a specific 2^N orthant.
    pure Maybe<I64> index(const Pos<N> &pos) const {
        return_if(!box().contains(pos), None);

        I64 flat_index = 0;
        I64 stride = 1 << N;
        for (U64 i = 0; i < N; ++i) {
            I64 idx = pos[i] >= origin[i] ? 1 : 0;
            return_if(idx < 0 || idx > 1, None);
            flat_index += (idx == 0 ? 0 : stride);
            stride = stride >> 1;
        }
        return flat_index;
    }

    Pos<N> origin;
    U64 size;
};

/**
 * @class Node
 * @brief A node within an RTree.
 */
template <U64 N, typename ItemRef>
struct Node : Orthants<N> {
    static constexpr U64 E = 1 << N; // Number of orthants, i.e. 2^N

    Node(Node *parent, const U64 id, const Pos<N> &origin, const U64 size)
        : Orthants<N>(origin, size), parent(parent), id(id) {}

    Node(const Node &) = delete;
    Node &operator=(const Node &) = delete;

    pure bool operator==(const Node &rhs) const { return id == rhs.id; }
    pure bool operator!=(const Node &rhs) const { return !(*this == rhs); }

    pure Node *get(const Pos<N> &pos) const {
        const auto index = get_index(pos);
        return index ? children[*index] : nullptr;
    }

    void remove_child(Node *child) {
        simd for (U64 i = 0; i < E; ++i) { children[i] *= static_cast<int64_t>(children[i] == child); }
    }

    pure bool has_child() const {
        for (U64 i = 0; i < E; ++i) {
            return_if(children[i] != nullptr, true);
        }
        return false;
    }

    pure bool empty() const { return !has_child() && list.empty(); }

    pure U64 depth() const {
        U64 depth = 0;
        Node *node = parent;
        while (node) {
            node = node->parent;
            depth += 1;
        }
        return depth;
    }

    Node *parent;
    U64 id;
    List<ItemRef> list;
    Tuple<E, Node *> children = Tuple<E, Node *>::fill(nullptr);
};

} // namespace detail

/**
 * @class RTree
 * @brief Data structure for storing volumes within an N-dimensional space with O(log(N)) lookup.
 *
 * @tparam N - Number of dimensions in the N-dimensional space.
 * @tparam Item - Value type being stored.
 * @tparam ItemRef - Type used for providing references to items held in this tree. Defaults to Ref<Item>.
 * @tparam kMaxEntries - Maximum number of entries per node. Defaults to 10.
 * @tparam kGridExpMin - Minimum node grid size (2 ^ min_grid_exp). Defaults to 2.
 */
template <U64 N, typename Item, typename ItemRef = Ref<Item>, U64 kMaxEntries = 10, U64 kGridExpMin = 2>
    requires trait::HasBBox<Item>
class RTree : public detail::Node<N, ItemRef> {
public:
    using Node = detail::Node<N, ItemRef>;

    static constexpr I64 kGridMin = 0x1 << kGridExpMin;

    struct Intersect : nvl::Intersect<N> {
        explicit Intersect(const nvl::Intersect<N> &init, ItemRef ref) : nvl::Intersect<N>(init), item(ref) {}
        ItemRef item;
    };

    /// Allows iteration over all elements in the tree, viewing them as ItemRef (type wrapper).
    struct item_iterator final : AbstractIteratorCRTP<item_iterator, ItemRef> {
        class_tag(item_iterator, AbstractIterator<ItemRef>);
        using ItemMap = Map<U64, std::unique_ptr<Item>>;

        template <View Type = View::kImmutable>
        pure static Iterator<ItemRef, Type> begin(const ItemMap &map) {
            return make_iterator<item_iterator, Type>(map.values_begin());
        }
        template <View Type = View::kImmutable>
        pure static Iterator<ItemRef, Type> end(const ItemMap &map) {
            return make_iterator<item_iterator, Type>(map.values_end());
        }

        explicit item_iterator(Iterator<std::unique_ptr<Item>> iter) : iter(iter) {}

        void increment() override {
            item = None;
            ++iter;
        }

        pure const ItemRef *ptr() override {
            if (!item.has_value()) {
                // Lazily set the item on dereference to avoid dereferencing an empty iterator
                item = ItemRef(iter->get());
            }
            return &item.value();
        }

        pure bool operator==(const item_iterator &rhs) const override { return iter == rhs.iter; }

    private:
        Maybe<ItemRef> item = None;
        Iterator<std::unique_ptr<Item>> iter;
    };

    static Box<N> bbox(const ItemRef &item) { return static_cast<const Item *>(item.ptr())->bbox(); }

    RTree() : Node(nullptr, -1, Pos<N>::fill(0), kGridExpMin) {}

    RTree(std::initializer_list<Item> items) : RTree() {
        for (const auto &item : items) {
            insert(item);
        }
    }

    RTree(std::initializer_list<ItemRef> items) : RTree() {
        for (const ItemRef &item : items) {
            insert(item);
        }
    }

    explicit RTree(Range<Item> items) : RTree() { insert(items); }
    explicit RTree(Range<ItemRef> items) : RTree() { insert(items); }

    /// Inserts a copy of the item into the tree.
    /// Returns a reference to the copy held by the tree.
    ItemRef insert(const Item &item) { return insert_over(item); }
    ItemRef insert(const ItemRef &item) { return insert_over(*item); }

    /// Inserts a copy of each item into the tree.
    RTree &insert(const Range<Item> &items) {
        for (const Item &item : items)
            insert_over(item);
        return *this;
    }
    RTree &insert(const Range<ItemRef> &items) {
        for (const ItemRef &item : items)
            insert_over(item.raw());
        return *this;
    }

    ItemRef take(std::unique_ptr<Item> item) { return take_over(std::move(item)); }

    /// Constructs a new item and adds it to this tree.
    /// Returns a reference to the new item held by the tree.
    template <typename T = Item, typename... Args>
    ItemRef emplace(Args &&...args) {
        return emplace_over<T>(std::forward<Args>(args)...);
    }

    /// Removes the matching item from the tree, if it exists.
    RTree &remove(const ItemRef &item) { return remove_over(item, bbox(item), true); }
    RTree &remove(Range<ItemRef> items) {
        for (const ItemRef &item : items)
            remove_over(item, bbox(item), true);
        return *this;
    }

    /// Registers the matching item as having moved from the previous volume `prev` to its current volume.
    /// Does nothing if no matching item exists in the tree.
    RTree &move(const ItemRef &item, const Box<N> &prev) { return move_from(item, prev); }

    /// Calls [func] on all existing nodes in the given volume. Traversal is depth-first preorder.
    template <typename VisitFunc> // Node* => WalkResult
    void preorder_walk_nodes(VisitFunc func) const {
        preorder_walk_nodes_in(bbox_, func);
    }

    template <typename VisitFunc> // Node* => WalkResult
    void preorder_walk_nodes_in(const Box<N> &box, VisitFunc func) const {
        List<Node *> frontier{this};
        while (!frontier.empty()) {
            Node *current = frontier.back();
            frontier.pop_back();
            const WalkResult result = func(current);
            return_if(result == WalkResult::kExit);
            if (result == WalkResult::kRecurse) {
                for (Node *child : current->children) {
                    if (child && box.overlaps(child->box())) {
                        frontier.push_back(child);
                    }
                }
            }
        }
    }

    /// Returns a set of all stored items in the given volume.
    pure expand Set<ItemRef> operator[](const Box<N> &box) const { return collect(box); }
    pure expand Set<ItemRef> operator[](const Pos<N> &pos) const { return collect(Box<N>::unit(pos)); }

    /// Returns the first item stored in the given volume, if one exists.
    pure expand Maybe<ItemRef> first(const Box<N> &box) const { return collect_first(box); }
    pure expand Maybe<ItemRef> first(const Pos<N> &pos) const { return collect_first(Box<N>::unit(pos)); }

    /// Returns the closest item which intersects with the line segment.
    /// Also returns the location and face of the intersection, if it exists.
    pure expand Maybe<Intersect> first(const Line<N> &line) const {
        return first_where(line, [](const Intersect &intersect) { return intersect.dist; });
    }

    /// Returns true if there are any items stored in the given volume.
    pure expand bool exists(const Box<N> &box) const { return collect_first(box).has_value(); }
    pure expand bool exists(const Pos<N> &pos) const { return collect_first(Box<N>::unit(pos)).has_value(); }

    /// Returns the closest item which intersects with the line segment according to the distance function.
    /// Also returns the location and face of the intersection, if it exists.
    template <typename DistanceFunc> // Intersect => Maybe<F64>
    pure Maybe<Intersect> first_where(const Line<N> &line, DistanceFunc dist) const {
        Maybe<Intersect> closest = None;
        Maybe<F64> distance = None;
        // TODO: Feels like we can improve this. Can potentially get a lot of volume which would not intersect.
        for (auto item : (*this)[{floor(line.a()), ceil(line.b())}]) {
            if (auto intersection = line.intersect(bbox(item))) {
                Intersect inter(*intersection, item);
                const Maybe<F64> len = dist(inter);
                if (len && (!distance.has_value() || *len < *distance)) {
                    distance = len;
                    closest = inter;
                }
            }
        }
        return closest;
    }

    /// Returns a Range for unordered iteration over all items in this tree.
    pure MRange<ItemRef> items() { return {begin(), end()}; }
    pure Range<ItemRef> items() const { return {begin(), end()}; }

    pure MIterator<ItemRef> begin() { return item_iterator::template begin<View::kMutable>(items_); }
    pure MIterator<ItemRef> end() { return item_iterator::template end<View::kMutable>(items_); }

    pure Iterator<ItemRef> begin() const { return item_iterator::template begin<View::kImmutable>(items_); }
    pure Iterator<ItemRef> end() const { return item_iterator::template end<View::kImmutable>(items_); }

    /// Returns true if this item is contained within the tree.
    pure bool has(const ItemRef &item) const { return item_ids_.has(item); }

    /// Returns the connected components in this tree.
    pure List<Set<ItemRef>> components() const {
        UnionFind<ItemRef> components;
        for (const std::unique_ptr<Item> &a : items_.values()) {
            ItemRef a_ref(a.get());
            bool had_neighbors = false;
            for (const auto &edge : a->bbox().edges()) {
                for (const ItemRef &b : collect(edge.bbox())) {
                    had_neighbors = true;
                    components.add(a_ref, b); // Adds neighboring boxes to the same component
                }
            }
            // Add this item to its own component if it had no neighbors
            if (!had_neighbors) {
                components.add(a_ref);
            }
        }
        // Need to return this as a List to avoid references to the local `components` data structure.
        List<Set<ItemRef>> result;
        for (const Set<ItemRef> &set : components.sets()) {
            result.push_back(std::move(set));
        }
        return result;
    }

    /// Returns the current bounding box for this tree, if defined.
    /// Returns an empty box otherwise.
    pure Box<N> bbox() const { return bbox_; }

    /// Returns the shape of the bounding box for this tree.
    pure Pos<N> shape() const { return bbox().shape(); }

    /// Returns the total number of distinct items stored in this tree.
    pure U64 size() const { return items_.size(); }

    /// Returns the total number of nodes in this tree.
    pure U64 nodes() const { return nodes_.size(); }

    /// Returns true if this tree is empty.
    pure bool empty() const { return items_.empty(); }

    /// Returns the maximum depth, in nodes, of this tree. O(N) with number of nodes in the tree.
    pure U64 depth() const {
        U64 depth = 0;
        for (auto &[_, node] : nodes_) {
            depth = std::max(depth, node.depth());
        }
        return depth;
    }

    void clear() {
        item_ids_.clear();
        nodes_.clear();
        items_.clear();
        node_id_ = 0;
        item_id_ = 0;
        bbox_ = Box<N>::kEmpty;
        root_ = nullptr;
    }

    /// Dumps a string representation of this tree to stdout.
    void dump() const {
        std::cout << "[[RTree with bounds " << bbox() << "]]" << std::endl;
        preorder_walk_nodes([&](Node *node) {
            const U64 indent = node->depth();
            indented(indent) << "[#" << node->id << "][" << node->grid << ": " << node->box() << "]:" << std::endl;
            for (const ItemRef &item : node->list) {
                indented(indent) << "> " << item << std::endl;
            }
            return WalkResult::kRecurse;
        });
    }

protected:
    struct Garbage {
        explicit Garbage(RTree *parent) : parent(parent) {}
        ~Garbage() {
            for (const Node *removed : removed_nodes) {
                parent->nodes_.erase(removed->id);
            }
        }
        List<Node *> removed_nodes;
        RTree *parent;
    };

    pure Set<ItemRef> collect(const Box<N> &box) const {
        Set<ItemRef> items;
        preorder_walk_nodes_in(box, [&](Node *node) {
            for (const ItemRef &item : node->list) {
                if (box.overlaps(bbox(item))) {
                    items.insert(item);
                }
            }
            return WalkResult::kRecurse;
        });
        return items;
    }

    pure Maybe<ItemRef> collect_first(const Box<N> &box) const {
        Maybe<ItemRef> result = None;
        preorder_walk_nodes_in(box, [&](Node *node) {
            for (const ItemRef &item : node->list) {
                if (box.overlaps(bbox(item))) {
                    result = item;
                    return WalkResult::kExit;
                }
            }
            return WalkResult::kRecurse;
        });
        return result;
    }

    Node *next_node(Node *parent, const I64 grid, const Pos<N> &min) {
        const U64 id = node_id_++;
        return &nodes_.emplace(std::piecewise_construct, std::forward_as_tuple(id),
                               std::forward_as_tuple(parent, id, grid, min));
    }

    /// Pushes list entries down if [node] has exceeded the maximum entries, creating new children when necessary.
    /// Returns the list of direct children of [node] that were updated.
    List<Node *> balance_only(Node *node) {
        return_if(node->grid <= kGridMin || node->list.size() < kMaxEntries, {}); // Can't further balance
        List<ItemRef> move;
        for (const auto &item : node->list) {
            auto box = bbox(item);
            auto min = box.shape().min();
            // TODO: Maybe this should be `min <= grid / 2`?
            if (min < node->grid) { // Only push down entries which are smaller than this node's granularity
                move.push_back(item);
            }
        }
        return_if(move.empty(), {});

        static constexpr Box<N> counter{Tuple<N, I64>::fill(0), Tuple<N, I64>::fill(2)};
        const I64 child_grid = node->grid / 2;

        U64 i = 0;
        List<Node *> updated;
        for (const Pos<N> &index : counter.indices()) {
            const Pos<N> child_min = node->min + index * child_grid;
            const Pos<N> child_end = child_min + child_grid;
            const Box<N> child_box{child_min, child_end};
            List<ItemRef> child_items;
            for (auto &item : move) {
                if (bbox(item).overlaps(child_box)) {
                    child_items.push_back(item);
                }
            }
            if (!child_items.empty()) {
                Node *child = node->children[i];
                if (child == nullptr) {
                    child = next_node(node, child_grid, child_min);
                }
                child->list.append(child_items);
                updated.push_back(child);
            }
            ++i;
        }
        return updated;
    }

    /// Recursively balances all nodes at and below [node], starting with [node].
    void balance(Node *node) {
        List<Node *> frontier{node};
        while (!frontier.empty()) {
            Node *current = frontier.back();
            frontier.pop_back();
            frontier.append(balance_only(current));
        }
    }

    /// Removes all empty nodes above and including [node].
    void remove_if_empty(Garbage &garbage, Node *node) {
        while (node) {
            Node *parent = node->parent;
            return_if(!node->empty() || !parent);
            garbage.removed_nodes.push_back(node);
            parent->remove_child(node);
            node = parent;
        }
    }

    /// Removes [item] from [node], if it exists.
    /// Then recursively removes any empty nodes above and including [node].
    bool remove(Garbage &garbage, Node *node, const ItemRef &item) {
        // TODO: O(N) with number of items here
        const bool changed = node->list.remove(item);
        if (changed) {
            remove_if_empty(garbage, node);
        }
        return changed;
    }

    Maybe<std::pair<U64, ItemRef>> get_item(const ItemRef &item) {
        if (auto iter = item_ids_.find(item); iter != item_ids_.end()) {
            return Some(std::pair<U64, ItemRef>{iter->second, iter->first});
        }
        return None;
    }

    void add_to_bbox(const Box<N> &added) {
        bbox_ = bounding_box(bbox_, added);
        const I64 max_shape = bbox_.shape().max();
        // Add one to the ceil(log2(shape)). Box may not be aligned with the grid, this should account for that.
        const I64 max_exp = std::max<I64>(ceil_log2(max_shape) + 1, kGridMin);

        if (!root_) {
            const U64 grid_max = 0x1 << max_exp;
            root_ = next_node(nullptr, grid_max, bbox_.clamp(grid_max).min);
        } else {
            // Rebalance the root to match the new desired maximum grid size. Skip if already sufficiently sized.
            const I64 root_exp = root_ ? ceil_log2(root_->grid) : kGridMin;
            return_if(root_exp >= max_exp);

            Node *prev = root_;
            for (I64 grid_exp = root_exp + 1; grid_exp <= max_exp; ++grid_exp) {
                const I64 grid_size = 0x1 << grid_exp;
                const Pos<N> grid_min = prev->min.grid_min(grid_size);
                root_ = next_node(nullptr, grid_size, grid_min);
                prev->parent = root_;
                const auto index = root_->get_index(prev->min);
                root_->children[*index] = prev;
                prev = root_;
            }
        }
    }

    void add_and_balance(const ItemRef &ref) {
        add_to_bbox(bbox(ref));
        root_->list.push_back(ref);
        balance(root_);
    }

    RTree &move_from(const ItemRef &item, const Box<N> &old_box) {
        if (auto pair = get_item(item)) {
            auto [_, ref] = *pair;
            Garbage garbage(this);
            remove_over(ref, old_box, /*remove_all*/ false);
            add_and_balance(ref);

            /*for (const Box<N> &added : new_box.diff(old_box)) {
                preorder_walk_nodes_in([&](Node *node) {

                });
                for (auto [node, pos] : points_in(added)) {
                    if (!old_box.overlaps({pos, pos + node->grid})) {
                        node->map[pos].list.emplace_back(ref);
                        balance(node, pos);
                    }
                }
            }*/
        }
        return *this;
    }

    ItemRef insert_over(const Item &item) {
        const U64 id = ++item_id_;
        auto &unique = items_[id] = std::make_unique<Item>(item); // Copy constructor
        ItemRef ref(unique.get());
        item_ids_[ref] = id;
        add_and_balance(ref);
        return ref;
    }

    ItemRef take_over(std::unique_ptr<Item> item) {
        const U64 id = ++item_id_;
        auto &unique = items_[id] = std::move(item);
        ItemRef ref(unique.get());
        item_ids_[ref] = id;
        add_and_balance(ref);
        return ref;
    }

    template <typename T, typename... Args>
    ItemRef emplace_over(Args &&...args) {
        const U64 id = ++item_id_;
        auto &unique = items_[id] = std::move(std::make_unique<T>(std::forward<Args>(args)...));
        ItemRef ref(unique.get());
        item_ids_[ref] = id;
        add_and_balance(ref);
        return ref;
    }

    RTree &remove_over(const ItemRef item, const Box<N> &box, const bool remove_all) {
        // TODO: Update bounds?
        if (auto pair = get_item(item)) {
            Garbage garbage(this);
            const ItemRef &ptr = pair->second;
            preorder_walk_nodes_in(box, [&](Node *node) {
                // Don't continue to recurse if item was found in this node
                return remove(garbage, node, ptr) ? WalkResult::kNoRecurse : WalkResult::kRecurse;
            });
            if (remove_all) {
                items_.remove(pair->first);
            }
        }
        return *this;
    }

    Box<N> bbox_ = Box<N>::kEmpty;
    U64 node_id_ = 0;
    U64 item_id_ = 0;

    // Nodes keep references to the items stored in the items_ map to avoid storing two copies of each item.
    // These references are guaranteed stable as long as the item itself is not removed from the map.
    // See: https://cplusplus.com/reference/unordered_map/unordered_map/operator[]/
    Map<U64, std::unique_ptr<Item>> items_;
    Map<ItemRef, U64> item_ids_;

    Map<U64, Node> nodes_;
    Node *root_ = nullptr;
};

} // namespace nvl
