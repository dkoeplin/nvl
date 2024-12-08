#pragma once

#include <memory>

#include "nvl/data/List.h"
#include "nvl/data/Map.h"
#include "nvl/data/Once.h"
#include "nvl/data/PointerHash.h"
#include "nvl/data/Range.h"
#include "nvl/data/Ref.h"
#include "nvl/data/Set.h"
#include "nvl/data/UnionFind.h"
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
 * @class Node
 * @brief A node within an RTree.
 */
template <U64 N, typename ItemRef>
struct Node {
    struct Parent {
        Node *node; // The parent node
        Box<N> box; // Bounds in reference to the parent's grid
    };

    struct Entry {
        enum Kind { kNode, kList };
        Kind kind = kList;
        Node *node = nullptr;
        List<ItemRef> list;
    };

    Node() = default;
    Node(const Node &) = delete;
    Node &operator=(const Node &) = delete;

    pure bool operator==(const Node &rhs) const { return id == rhs.id; }
    pure bool operator!=(const Node &rhs) const { return !(*this == rhs); }

    pure Entry *get(const Pos<N> &pos) const { return map.get(pos.grid_min(grid)); }

    pure Range<Pos<N>> indices(const Box<N> &vol) const { return vol.clamp(grid).indices(Pos<N>::fill(grid)); }

    U64 id = -1;
    Maybe<Parent> parent;
    I64 grid = -1;
    Map<Pos<N>, Entry> map;
};

template <U64 N, typename ItemRef>
struct Work {
    using Node = Node<N, ItemRef>;

    explicit Work(Node *node, const Box<N> &volume) : node(node), vol(volume) {}

    pure const Pos<N> &pos() const { return *pair_range.begin(); }
    pure const ItemRef &item() const { return *list_range; }

    Node *node;
    Box<N> vol;

    Once<ItemRef> list_range; // 1) Iterating across items
    Once<Pos<N>> pair_range;  // 2) Iterating within a node - (node, pos) pairs
};

template <U64 N, typename ItemRef>
struct PreorderWork {
    using Node = Node<N, ItemRef>;

    PreorderWork(Node *node, const U64 depth) : node(node), depth(depth) {
        ASSERT(node->parent.has_value(), "No parent defined for node #" << node->id);
        const Box<N> &node_range = node->parent->box;
        indented(depth - 1) << ">>[" << node->id << "] @ " << node->grid << std::endl;
        pos_range = node_range.indices(node->grid).once();
    }
    PreorderWork(Node *node, const Box<N> &bounds) : node(node), depth(0) {
        indented(depth) << "[" << node->id << "] @ " << node->grid << std::endl;
        pos_range = bounds.clamp(node->grid).indices(node->grid).once();
    }
    Node *node;
    U64 depth;
    Once<Pos<N>> pos_range;
};

} // namespace detail

/**
 * @class RTree
 * @brief Data structure for storing volumes within an N-dimensional space with O(log(N)) lookup.
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
class RTree {
public:
    using PreorderWork = detail::PreorderWork<N, ItemRef>;
    using Work = detail::Work<N, ItemRef>;
    using Node = detail::Node<N, ItemRef>;
    using ItemRefHash = PointerHash<ItemRef>; // Hashing is done based on pointer, not value
    using Component = typename UnionFind<ItemRef, ItemRefHash>::Group;

    struct Point {
        Node *node;
        Pos<N> pos;
    };

    enum class Traversal {
        kPoints,  // All possible points in existing nodes
        kEntries, // All existing entries
        kItems    // All existing items
    };
    template <typename Concrete, Traversal mode, typename Value>
    abstract struct abstract_iterator : AbstractIteratorCRTP<Concrete, Value> {
        class_tag(abstract_iterator, AbstractIterator<Value>);

        template <View Type = View::kImmutable>
        static Iterator<Value, Type> begin(const RTree &tree, const Box<N> &box) {
            std::shared_ptr<Concrete> iter = std::make_shared<Concrete>(&tree, box);
            if (tree.root_ != nullptr) {
                iter->worklist.emplace_back(tree.root_, box);
                iter->increment();
            }
            return Iterator<Value, Type>(std::move(iter));
        }

        template <View Type = View::kImmutable>
        static Iterator<Value, Type> end(const RTree &tree, const Box<N> &box) {
            return make_iterator<Concrete, Type>(&tree, box);
        }

        explicit abstract_iterator(const RTree *tree, const Box<N> &box) : tree(tree), box(box) {}

        bool skip_item(Work &current) {
            ItemRef ref = current.item();
            return visited.has(ref) || !bbox(ref).overlaps(box);
        }

        HOT bool visit_next_pair(Work &current) {
            Node *node = current.node;

            if (!current.pair_range.has_next()) {
                worklist.pop_back();
                return false;
            }

            const Pos<N> &pos = current.pos();

            if (auto *entry = node->get(pos)) {
                if (entry->kind == Node::Entry::kList) {
                    if constexpr (mode == Traversal::kItems) {
                        current.list_range = {entry->list.begin(), entry->list.end()};
                        while (current.list_range.has_next() && skip_item(current)) {
                            ++current.list_range;
                        }
                        if (current.list_range.has_next()) {
                            // Start visiting this list if there is at least one valid item
                            visited.insert(current.item());
                            return true;
                        }
                        // Skip this list entirely if it had no new unique items
                        return false;
                    }
                    // Visit this list or (node, pos)
                    return true;
                }
                ASSERT(entry->node->parent.has_value(), "Sub-node had no parent entry.");
                if (auto range = entry->node->parent->box.intersect(box)) {
                    // Continue to this child node by updating the worklist
                    worklist.emplace_back(entry->node, *range);
                }
            } else {
                // Visit this (currently unset) (node, pos) pair
                if constexpr (mode == Traversal::kPoints) {
                    return true;
                }
            }
            return false;
        }

        bool advance_list(Work &current) {
            do {
                ++current.list_range;
            } while (current.list_range.has_next() && skip_item(current));

            if (current.list_range.has_next()) {
                visited.insert(current.item());
                return true;
            }
            return false;
        }

        bool advance_pair(Work &current) {
            ++current.pair_range;
            return visit_next_pair(current);
        }

        bool advance_node(Work &current) {
            current.pair_range = current.node->indices(current.vol).once();
            return visit_next_pair(current);
        }

        /// Advances this iterator to either the next list item, next (node, pos) pair, or next child node.
        void increment() override {
            while (!worklist.empty()) {
                auto &current = worklist.back();
                if (current.list_range.has_next()) {
                    return_if(advance_list(current));
                } else if (current.pair_range.has_next()) {
                    return_if(advance_pair(current));
                } else {
                    return_if(advance_node(current));
                }
            }
        }

        pure bool operator==(const Concrete &rhs) const override {
            return tree == rhs.tree && worklist.get_back() == rhs.worklist.get_back();
        }

        // 3) Iterating across nodes
        List<Work> worklist = {};
        Set<ItemRef, ItemRefHash> visited;

        const RTree *tree;
        Box<N> box;
    };

    struct entry_iterator final : abstract_iterator<entry_iterator, Traversal::kEntries, Point> {
        class_tag(entry_iterator, abstract_iterator<entry_iterator, Traversal::kEntries, Point>);
        using parent = abstract_iterator<entry_iterator, Traversal::kEntries, Point>;
        using parent::parent;

        const Point *ptr() override {
            if (!point.has_value()) {
                auto &current = this->worklist.back();
                point = {current.node, current.pos()};
            }
            return &point.value();
        }

        void increment() override {
            parent::increment();
            point = None;
        }

        Maybe<Point> point = None;
    };

    struct point_iterator final : abstract_iterator<point_iterator, Traversal::kPoints, Point> {
        class_tag(point_iterator, abstract_iterator<point_iterator, Traversal::kPoints, Point>);
        using parent = abstract_iterator<point_iterator, Traversal::kPoints, Point>;
        using parent::parent;

        const Point *ptr() override {
            if (!point.has_value()) {
                auto &current = this->worklist.back();
                point = {current.node, current.pos()};
            }
            return &point.value();
        }

        void increment() override {
            parent::increment();
            point = None;
        }

        Maybe<Point> point = None;
    };

    struct window_iterator final : abstract_iterator<window_iterator, Traversal::kItems, ItemRef> {
        class_tag(window_iterator, abstract_iterator<window_iterator, Traversal::kItems, ItemRef>);
        using parent = abstract_iterator<window_iterator, Traversal::kItems, ItemRef>;
        using parent::parent;
        const ItemRef *ptr() override { return &this->worklist.back().item(); }
    };

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
    static bool should_increase_depth(const U64 size, const U64 grid) { return size > kMaxEntries && grid > grid_min; }

    static constexpr I64 grid_min = 0x1 << kGridExpMin;
    static constexpr I64 grid_max = 0x1 << kGridExpMax;

    RTree() : root_(next_node(None, grid_max, {})) {}

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
    ItemRef insert(const Item &item) { return insert_over(item, item.bbox()); }
    ItemRef insert(const ItemRef &item) { return insert_over(*item, bbox(item)); }

    /// Inserts a copy of each item into the tree.
    RTree &insert(const Range<Item> &items) {
        for (const Item &item : items)
            insert_over(item, item.bbox());
        return *this;
    }

    RTree &insert(const Range<ItemRef> &items) {
        for (const ItemRef &item : items)
            insert_over(item.raw(), item->bbox());
        return *this;
    }

    ItemRef take(std::unique_ptr<Item> item) {
        const Box<N> box = item->bbox();
        return take_over(std::move(item), box);
    }

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
    RTree &move(const ItemRef &item, const Box<N> &prev) { return move(item, bbox(item), prev); }

    /// Returns a mutable range over all _possible_ points in the given volume, including those without Nodes.
    MRange<Point> points_in(const Box<N> &box) { return make_range<point_iterator, View::kMutable>(*this, box); }

    /// Returns a mutable range over all existing nodes in the given volume.
    MRange<Point> entries_in(const Box<N> &box) { return make_range<entry_iterator, View::kMutable>(*this, box); }

    /// Returns an iterable range over all unique stored items in the given volume.
    pure MRange<ItemRef> operator[](const Box<N> &box) { return make_mrange<window_iterator>(*this, box); }
    pure Range<ItemRef> operator[](const Box<N> &box) const { return make_range<window_iterator>(*this, box); }

    /// Returns an iterable range over all unique stored items at the given point.
    pure MRange<ItemRef> operator[](const Pos<N> &pt) { return operator[](Box<N>::unit(pt)); }
    pure Range<ItemRef> operator[](const Pos<N> &pt) const { return operator[](Box<N>::unit(pt)); }

    struct Intersect : nvl::Intersect<N> {
        explicit Intersect(const nvl::Intersect<N> &init, ItemRef ref) : nvl::Intersect<N>(init), item(ref) {}
        ItemRef item;
    };

    /// Returns the closest item which intersects with the line segment according to the distance function.
    pure Maybe<Intersect> first_where(const Line<N> &line, const std::function<Maybe<F64>(Intersect)> &dist) const {
        Maybe<Intersect> closest = None;
        Maybe<F64> distance = None;
        // TODO: Feels like we can improve this. Can potentially get a lot of volume which would not intersect.
        for (auto item : (*this)[{floor(line.a()), ceil(line.b())}]) {
            if (auto intersection = line.intersect(bbox(item))) {
                Intersect inter(*intersection, item);
                if (auto len = dist(inter); len && (!distance.has_value() || *len < *distance)) {
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

    pure Iterator<ItemRef> begin() const { return item_iterator::template begin(items_); }
    pure Iterator<ItemRef> end() const { return item_iterator::template end(items_); }

    /// Returns true if this item is contained within the tree.
    pure bool has(const ItemRef &item) const { return item_ids_.has(item); }

    /// Returns the connected components in this tree.
    List<Component> components() {
        UnionFind<ItemRef, ItemRefHash> components;
        for (const std::unique_ptr<Item> &a : items_.values()) {
            ItemRef a_ref(a.get());
            bool had_neighbors = false;
            for (const auto &edge : a->bbox().edges()) {
                for (const ItemRef &b : (*this)[edge.bbox()]) {
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
        List<Component> result;
        for (const Component &set : components.sets()) {
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

    /// Returns the maximum depth, in nodes, of this tree.
    pure U64 depth() const {
        const U64 root_grid = root_->grid;
        U64 min_grid = root_grid;
        for (auto &[_, node] : nodes_) {
            min_grid = (node.grid < min_grid) ? node.grid : min_grid;
        }
        return ceil_log2(root_grid) - ceil_log2(min_grid) + 1;
    }

    void clear() {
        item_ids_.clear();
        nodes_.clear();
        items_.clear();
        node_id_ = 0;
        item_id_ = 0;
        bbox_ = Box<N>::kEmpty;
        root_ = next_node(None, grid_max, {});
    }

    /// Dumps a string representation of this tree to stdout.
    void dump() const {
        const auto bounds = bbox();
        std::cout << "[[RTree with bounds " << bounds << "]]" << std::endl;

        List<PreorderWork> worklist;
        worklist.emplace_back(root_, bounds);

        while (!worklist.empty()) {
            PreorderWork &current = worklist.back();
            Node *node = current.node;
            const U64 depth = current.depth;
            bool found_node = false;
            while (!found_node && current.pos_range.has_next()) {
                Once<Pos<N>> &iter = current.pos_range;
                const Pos<N> pos = *iter;
                ++iter;
                if (auto *entry = node->get(pos)) {
                    const Box<N> range(pos, pos + node->grid);
                    indented(depth) << "[" << node->id << "][" << range << "]:" << std::endl;
                    if (entry->kind == Node::Entry::kList) {
                        if (entry->list.empty()) {
                            indented(depth) << ">> EMPTY LIST" << std::endl;
                        }
                        for (const auto item : entry->list) {
                            indented(depth) << ">> " << item << std::endl;
                        }
                    } else {
                        worklist.emplace_back(entry->node, depth + 1);
                        found_node = true;
                    }
                }
            }
            if (!worklist.back().pos_range.has_next()) {
                worklist.pop_back();
            }
        }
    }

private:
    struct Garbage {
        explicit Garbage(RTree *parent) : parent(parent) {}
        ~Garbage() {
            for (const U64 removed_id : removed_nodes) {
                parent->nodes_.erase(removed_id);
            }
        }
        List<U64> removed_nodes;
        RTree *parent;
    };

    Node *next_node(const Maybe<typename Node::Parent> &parent, const I64 grid, const List<ItemRef> &items) {
        const Pos<N> grid_fill = Pos<N>::fill(grid);
        const U64 id = node_id_++;
        Node *node = &nodes_.emplace(std::piecewise_construct, std::forward_as_tuple(id), std::tuple{});
        node->parent = parent;
        node->id = id;
        node->grid = grid;
        for (const ItemRef &item : items) {
            const Box<N> item_box = bbox(item);
            Range<Box<N>> points; // empty iterable to start with
            if (parent.has_value()) {
                if (const Maybe<Box<N>> intersection = item_box.intersect(parent->box)) {
                    points = intersection->clamp(grid_fill).volumes(grid_fill);
                }
            } else {
                points = item_box.clamp(grid_fill).volumes(grid_fill);
            }
            for (const Box<N> &dest : points) {
                if (dest.overlaps(item_box)) {
                    typename Node::Entry &entry = node->map.get_or_add(dest.min, {});
                    entry.list.push_back(item);
                }
            }
        }
        // Re-balance the newly created node. This may create more nodes!
        balance(node);
        return node;
    }

    void balance_pos(Node *node, const Pos<N> &pos) {
        if (auto *entry = node->get(pos)) {
            if (entry->kind == Node::Entry::kList) {
                if (should_increase_depth(entry->list.size(), node->grid)) {
                    const Box<N> child_box(pos, pos + node->grid);
                    const I64 child_grid = node->grid / 2;
                    const typename Node::Parent parent{.node = node, .box = child_box};
                    entry->node = next_node(parent, child_grid, entry->list);
                    entry->kind = Node::Entry::kNode;
                    entry->list.clear();
                }
            } else if (entry->kind == Node::Entry::kNode) {
                balance(entry->node);
            }
        }
    }

    void balance(Node *node) {
        return_if(node->grid <= grid_min); // Can't further balance
        for (auto &[pos, _] : node->map) {
            balance_pos(node, pos);
        }
    }

    void balance(Node *node, const Pos<N> &pos) {
        return_if(node->grid <= grid_min); // Can't further balance
        balance_pos(node, pos);
    }

    void remove(Garbage &garbage, Node *node, const Pos<N> &pos) {
        if (auto *entry = node->get(pos)) {
            if (entry->kind == Node::Entry::kNode) {
                Node *child = entry->node;
                garbage.removed_nodes.push_back(child->id);
            }
            node->map.remove(pos);
        }
        if (node->map.empty() && node->parent.has_value()) {
            remove(garbage, node->parent->node, node->parent->box.min);
        }
    }

    void remove(Garbage &garbage, Node *node, const Pos<N> &pos, const ItemRef &item) {
        if (auto *entry = node->get(pos)) {
            ASSERT(entry->kind == Node::Entry::kList, "Cannot remove from non-list entry");
            entry->list.remove(item); // TODO: O(N) with number of items here
            if (entry->list.empty()) {
                remove(garbage, node, pos);
            }
        }
    }

    Maybe<std::pair<U64, ItemRef>> get_item(const ItemRef &item) {
        if (auto iter = item_ids_.find(item); iter != item_ids_.end()) {
            return Some(std::pair<U64, ItemRef>{iter->second, iter->first});
        }
        return None;
    }

    RTree &move(const ItemRef &item, const Box<N> &new_box, const Box<N> &old_box) {
        bbox_ = bounding_box(bbox_, new_box);
        if (auto pair = get_item(item)) {
            auto [_, ref] = *pair;
            Garbage garbage(this);
            for (const Box<N> &removed : old_box.diff(new_box)) {
                for (auto [node, pos] : entries_in(removed)) {
                    if (!new_box.overlaps({pos, pos + node->grid})) {
                        remove(garbage, node, pos, ref);
                    }
                }
            }
            for (const Box<N> &added : new_box.diff(old_box)) {
                for (auto [node, pos] : points_in(added)) {
                    if (!old_box.overlaps({pos, pos + node->grid})) {
                        node->map[pos].list.emplace_back(ref);
                        balance(node, pos);
                    }
                }
            }
        }
        return *this;
    }

    void populate_over(const ItemRef &ref, const Box<N> &box) {
        for (auto [node, pos] : points_in(box)) {
            node->map[pos].list.emplace_back(ref);
            balance(node, pos);
        }
    }

    ItemRef insert_over(const Item &item, const Box<N> &box) {
        bbox_ = bounding_box(bbox_, box);
        const U64 id = ++item_id_;
        auto &unique = items_[id] = std::make_unique<Item>(item); // Copy constructor
        ItemRef ref(unique.get());
        item_ids_[ref] = id;
        populate_over(ref, box);
        return ref;
    }

    ItemRef take_over(std::unique_ptr<Item> item, const Box<N> &box) {
        bbox_ = bounding_box(bbox_, box);
        const U64 id = ++item_id_;
        auto &unique = items_[id] = std::move(item);
        ItemRef ref(unique.get());
        item_ids_[ref] = id;
        populate_over(ref, box);
        return ref;
    }

    template <typename T, typename... Args>
    ItemRef emplace_over(Args &&...args) {
        const U64 id = ++item_id_;
        auto &unique = items_[id] = std::move(std::make_unique<T>(std::forward<Args>(args)...));
        bbox_ = bounding_box(bbox_, unique->bbox());
        ItemRef ref(unique.get());
        item_ids_[ref] = id;
        populate_over(ref, unique->bbox());
        return ref;
    }

    RTree &remove_over(const ItemRef item, const Box<N> &box, const bool remove_all) {
        if (auto pair = get_item(item)) {
            // TODO: Update bounds?
            Garbage garbage(this);
            for (auto [node, pos] : entries_in(box)) {
                remove(garbage, node, pos, pair->second);
            }
            if (remove_all) {
                items_.remove(pair->first);
            }
        }
        return *this;
    }

    Box<N> bbox_ = Box<N>::kEmpty;
    U64 node_id_ = 0;
    U64 item_id_ = 0;

    // Nodes keep references to the items stored in the above map to avoid storing two copies of each item.
    // These references are guaranteed stable as long as the item itself is not removed from the map.
    // See: https://cplusplus.com/reference/unordered_map/unordered_map/operator[]/
    Map<U64, std::unique_ptr<Item>> items_;
    Map<ItemRef, U64, ItemRefHash> item_ids_;

    Map<U64, Node> nodes_;
    Node *root_;
};

} // namespace nvl
