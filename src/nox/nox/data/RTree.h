#pragma once

#include "nox/data/List.h"
#include "nox/data/Map.h"
#include "nox/data/Ref.h"
#include "nox/data/Set.h"
#include "nox/geo/Box.h"
#include "nox/geo/Pos.h"
#include "nox/macros/Aliases.h"
#include "nox/macros/ReturnIf.h"

namespace nox {

template <U64 N, typename Value> struct get_box {
    virtual ~get_box() = default;
    pure virtual Box<N> operator()(const Value &value) const { return value.box; }
};

template <typename Value> struct get_id {
    virtual ~get_id() = default;
    pure virtual U64 operator()(const Value &value) const { return value.id; }
};

// Needs to be defined outside the class due to some clang oddities
struct RTreeOptions {
    /// Maximum number of entries per node. Nodes are split above this threshold.
    U64 max_entries = 10;

    /// Numeric base used for the grid size when creating nodes.
    U64 grid_base = 2;

    /// Minimum grid size (grid_base ^ min_grid_exp). Nodes are not split below this threshold.
    U64 min_grid_exp = 2;

    /// Initial grid size of the root. (grid_base ^ root_grid_exp)
    U64 root_grid_exp = 10;
};

template <U64 N,                               // Number of dimensions for each value
          typename Value,                      // Value type
          typename GetBox = get_box<N, Value>, // Fetches the associated volume for a value
          typename GetID = get_id<Value>>      // Fetches the associated ID for a value
class RTree {
  public:
    using Options = RTreeOptions;
    explicit RTree(const Options &opts = Options())
        : options_(opts), grid_min_(std::pow(options_.grid_base, options_.min_grid_exp)) {

        root_ = next_node(None, std::pow(options_.grid_base, options_.root_grid_exp), {});
    }
    explicit RTree(const List<Value> &values, Options opts = Options()) : RTree(opts) {
        for (const auto &value : values) {
            insert(value);
        }
    }

  private:
    friend struct Node;
    friend struct Testing;

    struct Node {
        struct Parent {
            Node *node; // The parent node
            Box<N> box; // Bounds in reference to the parent's grid
        };

        struct Entry {
            enum Kind { kNode, kList };
            Kind kind = kList;
            Node *node = nullptr;
            List<Ref<Value>> list;
        };

        struct iterator {};

        Node() = default;
        Node(const Node &) = delete;
        Node &operator=(const Node &) = delete;
        Node &operator=(Node &&) = delete;

        pure Entry *get(const Pos<N> &pos) const { return map.get(pos.clamp_down(grid)); }

        void init_list(const Pos<N> &pos, Value &value) { map[pos].list.emplace_back(value); }

        void insert(const Pos<N> &pos, Ref<Value> value) {
            map[pos].list.emplace_back(value);
            balance(pos);
        }

        void balance_pos(const Pos<N> &pos) {
            if (Entry *entry = get(pos)) {
                if (entry->kind == Entry::kList) {
                    if (tree->should_increase_depth(entry->list.size(), grid)) {
                        const Box<N> child_box(pos, pos + grid);
                        const U64 child_grid = grid / tree->options_.grid_base;
                        entry->node = tree->next_node(Parent{.node = this, .box = child_box}, child_grid, entry->list);
                        entry->kind = Entry::kNode;
                        entry->list.clear();
                    }
                } else if (entry->kind == Entry::kNode) {
                    entry->node->balance();
                }
            }
        }

        void balance() {
            if (grid <= tree->grid_min_)
                return; // Can't further balance

            for (auto &[pos, _] : map.unordered()) {
                balance_pos(pos);
            }
        }

        void balance(const Pos<N> &pos) {
            if (grid <= tree->grid_min_)
                return; // Can't further balance
            balance_pos(pos);
        }

        void remove(const Pos<N> &pos) {
            if (Entry *entry = get(pos)) {
                if (entry->kind == Entry::kNode) {
                    Node *child = entry->node;
                    tree->garbage_.push_back(child->id);
                }
                map.remove(pos);
            }
            if (map.is_empty() && parent.has_value()) {
                parent->node->remove(parent->box.min);
            }
        }

        void remove(const Pos<N> &pos, const Ref<Value> value) {
            if (Entry *entry = get(pos)) {
                ASSERT(entry->kind == Entry::kList, "Cannot remove from non-list entry");
                entry->list.remove(value); // TODO: O(N) with number of values here
                if (entry->list.is_empty()) {
                    remove(pos);
                }
            }
        }

        pure typename Box<N>::pos_range pos_iter(const Box<N> &vol) const {
            return vol.clamp(grid).pos_iter(Pos<N>::fill(grid));
        }

        U64 id = -1;
        Maybe<Parent> parent;
        U64 grid = -1;
        Map<Pos<N>, Entry> map;
        RTree *tree = nullptr;
    };

    struct Work {
        explicit Work(Node *node, const Box<N> &volume) : node(node), vol(volume) {}

        pure std::pair<Node *, Pos<N>> pair() const {
            ASSERT(node != nullptr && pair_iter.has_value(), "Attempted to dereference an empty iterator.");
            return {node, *pair_iter.value()};
        }

        pure Value &value() const {
            ASSERT(node != nullptr && list_iter.has_value(), "Attempted to dereference an empty iterator.");
            return list_iter.value()->raw();
        }

        bool operator==(const Work &rhs) const {
            return node == rhs.node && vol == rhs.vol && list_iter == rhs.list_iter && pair_iter == rhs.pair_iter;
        }
        bool operator!=(const Work &rhs) const { return !(*this == rhs); }

        Node *node;
        Box<N> vol;

        // 1) Iterating across values
        Maybe<typename List<Ref<Value>>::iterator> list_iter = None;
        Maybe<typename List<Ref<Value>>::iterator> list_end = None;

        // 2) Iterating within a node - (node, pos) pairs
        Maybe<typename Box<N>::pos_iterator> pair_iter = None;
        Maybe<typename Box<N>::pos_iterator> pair_end = None;
    };

    enum class Traversal {
        kPoints,  // All possible points in existing nodes
        kEntries, // All existing entries
        kValues   // All existing values
    };
    template <Traversal mode, typename Concrete> class abstract_iterator {
      public:
        Concrete &operator++() {
            advance();
            return *static_cast<Concrete *>(this);
        }

        pure bool operator==(const abstract_iterator &rhs) const {
            return &tree == &rhs.tree && worklist.get_back() == rhs.worklist.get_back();
        }
        pure bool operator!=(const abstract_iterator &rhs) const { return !(*this == rhs); }

      protected:
        friend class RTree;

        static Concrete begin(RTree &tree, const Box<N> &box) {
            Concrete iter(tree, box);
            if (tree.root_ != nullptr) {
                iter.worklist.emplace_back(tree.root_, box);
                iter.advance();
            }
            return iter;
        }

        static Concrete end(RTree &tree, const Box<N> &box) { return Concrete(tree, box); }

        explicit abstract_iterator(RTree &tree, const Box<N> &box) : tree(tree), box(box) {}

        bool visit_next_pair(Work &current) {
            Node *node = current.node;

            if (current.pair_iter == current.pair_end) {
                worklist.pop_back();
                return false;
            }

            const Pos<N> &pos = *current.pair_iter.value();

            if (typename Node::Entry *entry = node->get(pos)) {
                if (entry->kind == Node::Entry::kList) {
                    if constexpr (mode == Traversal::kValues) {
                        current.list_iter = entry->list.begin();
                        current.list_end = entry->list.end();
                        while (current.list_iter != current.list_end && visited.count(current.value().id)) {
                            current.list_iter->operator++();
                        }
                        if (current.list_iter != current.list_end) {
                            // Start visiting this list if there is at least one valid value
                            visited.insert(current.value().id);
                            return true;
                        }
                        // Skip this list entirely if it had no new unique values
                        current.list_iter = current.list_end = None;
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
                current.list_iter->operator++();
            } while (current.list_iter != current.list_end && visited.contains(current.value().id));

            if (current.list_iter != current.list_end) {
                visited.insert(current.value().id);
                return true;
            }
            current.list_iter = current.list_end = None;
            return false;
        }

        /// Advances the current (node, pos) to either the next (node, pos), the next list, or the next child node.
        bool advance_pair(Work &current) {
            current.pair_iter->operator++();
            return visit_next_pair(current);
        }

        bool advance_node(Work &current) {
            auto pts = current.node->pos_iter(current.vol);
            current.pair_iter = pts.begin();
            current.pair_end = pts.end();
            return visit_next_pair(current);
        }

        void advance() {
            while (!worklist.is_empty()) {
                auto &current = worklist.back();
                if (current.list_iter != None) {
                    return_if(advance_list(current));
                } else if (current.pair_iter != None) {
                    return_if(advance_pair(current));
                } else {
                    return_if(advance_node(current));
                }
            }
        }

        // 3) Iterating across nodes
        List<Work> worklist = {};
        Set<U64> visited = {};

        RTree &tree;
        Box<N> box;
    };

    template <typename Iterator> class abstract_range {
      public:
        pure Iterator begin() const { return Iterator::begin(tree, box); }
        pure Iterator end() const { return Iterator::end(tree, box); }

      private:
        friend class RTree;
        explicit abstract_range(RTree &tree, const Box<N> &box) : tree(tree), box(box) {}
        RTree &tree;
        Box<N> box;
    };

    class entry_iterator : public abstract_iterator<Traversal::kEntries, entry_iterator> {
      public:
        using Parent = abstract_iterator<Traversal::kEntries, entry_iterator>;
        using value_type = std::pair<Node *, Pos<N>>;
        using difference_type = std::ptrdiff_t;

        entry_iterator() = delete;

        value_type operator*() const {
            ASSERT(!this->worklist.is_empty(), "Attempted to dereference an empty iterator");
            return this->worklist.back().pair();
        }
        value_type operator->() const {
            ASSERT(!this->worklist.is_empty(), "Attempted to dereference an empty iterator");
            return this->worklist.back().pair();
        }

      private:
        friend class RTree;
        friend class entry_iterable;
        using Parent::begin;
        using Parent::end;
        using Parent::Parent;
    };

    class point_iterator : public abstract_iterator<Traversal::kPoints, point_iterator> {
      public:
        using Parent = abstract_iterator<Traversal::kPoints, point_iterator>;
        using value_type = std::pair<Node *, Pos<N>>;
        using difference_type = std::ptrdiff_t;

        point_iterator() = delete;

        value_type operator*() const {
            ASSERT(!this->worklist.is_empty(), "Attempted to dereference an empty iterator");
            return this->worklist.back().pair();
        }
        value_type operator->() const {
            ASSERT(!this->worklist.is_empty(), "Attempted to dereference an empty iterator");
            return this->worklist.back().pair();
        }

      private:
        friend class RTree;
        friend class entry_iterable;
        using Parent::begin;
        using Parent::end;
        using Parent::Parent;
    };

    using entry_range = abstract_range<entry_iterator>;
    using point_range = abstract_range<point_iterator>;

  public:
    class value_iterator : public abstract_iterator<Traversal::kValues, value_iterator> {
      public:
        using Parent = abstract_iterator<Traversal::kValues, value_iterator>;
        using pointer = Value &;
        using reference = Value &;
        using value_type = Value;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::input_iterator_tag;

        value_iterator() = delete;

        Value &operator*() const {
            ASSERT(!this->worklist.is_empty(), "Attempted to dereference empty iterator");
            return this->worklist.back().value();
        }
        Value &operator->() const {
            ASSERT(!this->worklist.is_empty(), "Attempted to dereference empty iterator");
            return this->worklist.back().value();
        }

      private:
        friend class RTree;
        using Parent::begin;
        using Parent::end;
        using Parent::Parent;
    };

    using value_range = abstract_range<value_iterator>;

    /// Inserts the value into the tree.
    RTree &insert(const Value &value) { return insert_over(value, value.box, value.id, true); }

    /// Removes the value from the tree.
    RTree &remove(const Value &value) { return remove_over(value.box, value.id, true); }

    /// Register a value as having moved from the previous volume `prev` to its current volume.
    RTree &move(const Value &value, const Box<N> &prev) { return move(value.box, value.id, prev); }

    /// Returns an iterator over all unique stored values in the given `volume`.
    pure value_range operator[](const Box<N> &volume) { return value_range(*this, volume); }

    /// Returns the current bounding box for this tree.
    pure const Box<N> &bounds() const {
        if (bounds_.has_value())
            return bounds_.value();
        return Box<N>::kUnitBox;
    }

    /// Returns the total number of nodes in this tree.
    pure U64 nodes() const { return nodes_.size(); }

    /// Returns the total number of distinct values stored in this tree.
    pure U64 size() const { return values_.size(); }

    // TODO: Unsafe - nondeterministic order
    pure typename Map<U64, Value>::const_iterator begin() const { return values_.begin(); }
    pure typename Map<U64, Value>::const_iterator end() const { return values_.end(); }

    struct Testing {
        static std::ostream &indented(U64 n) {
            for (U64 i = 0; i < n; i++) {
                std::cout << "    ";
            }
            return std::cout;
        }

        explicit Testing(RTree &tree) : tree(tree) {}

        void dump() const {
            const auto bounds = tree.bounds_.value_or(Box<N>::unit(Pos<N>::fill(1)));
            std::cout << "[[RTree with bounds " << bounds << "]]" << std::endl;

            List<std::pair<Node *, U64>> worklist;
            worklist.emplace_back(tree.root_, 0);
            while (!worklist.is_empty()) {
                auto [node, depth] = worklist.back();
                worklist.pop_back();

                const Box<N> node_range = node->parent.has_value() ? node->parent->box : bounds.clamp(node->grid);
                indented(depth) << "[" << depth << "] " << node_range << ": " << node->grid << std::endl;

                for (const Pos<N> &pos : node_range.pos_iter(node->grid)) {
                    if (auto *entry = node->get(pos)) {
                        const Box<N> range{pos, pos + node->grid - 1};
                        indented(depth) << "> " << range << ": " << std::endl;
                        if (entry->kind == Node::Entry::kList) {
                            for (const Ref<Value> value : entry->list) {
                                indented(depth) << ">> " << value << std::endl;
                            }
                        } else {
                            worklist.emplace_back(entry->node, depth + 1);
                        }
                    }
                }
            }
        }

        pure Map<Box<N>, Set<U64>> collect_ids() const {
            Map<Box<N>, Set<U64>> ids;
            for (const auto &[node, pos] : tree.entries_in(tree.bounds_.value_or(Box<N>::kUnitBox))) {
                if (auto *entry = node->get(pos); entry && entry->kind == Node::Entry::kList) {
                    const Box<N> box(pos, pos + node->grid - 1);
                    for (const auto &value : entry->list) {
                        ids[box].insert(value->id);
                    }
                }
            }
            return ids;
        }

        RTree &tree;
    };
    pure Testing testing() { return Testing(*this); }

  private:
    Node *next_node(const Maybe<typename Node::Parent> parent, const U64 grid, const List<Ref<Value>> &values) {
        const Pos<N> grid_fill = Pos<N>::fill(grid);
        const U64 id = ++node_id_;
        Node &node = nodes_[id];
        node.parent = parent;
        node.id = id;
        node.grid = grid;
        node.tree = this;
        for (const Ref<Value> value : values) {
            const Box<N> &value_box = value->box;
            typename Box<N>::box_range points; // empty iterable to start with
            if (parent.has_value()) {
                if (const Maybe<Box<N>> intersection = value_box.intersect(parent->box)) {
                    points = intersection->clamp(grid_fill).box_iter(grid_fill);
                }
            } else {
                points = value_box.clamp(grid_fill).box_iter(grid_fill);
            }
            for (const Box<N> &range : points) {
                if (range.overlaps(value_box)) {
                    typename Node::Entry &entry = node.map.get_or_add(range.min, {});
                    entry.list.push_back(value);
                }
            }
        }
        // Re-balance the newly created node. This may create more nodes!
        node.balance();
        return &node;
    }

    pure bool should_increase_depth(const U64 size, const U64 grid) const {
        return size > options_.max_entries && grid > grid_min_;
    }

    point_range points_in(const Box<N> &box) { return point_range(*this, box); }
    entry_range entries_in(const Box<N> &box) { return entry_range(*this, box); }

    RTree &move(const Box<N> &new_box, const U64 id, const Box<N> &prev_box) {
        const Value &value = values_[id];
        for (const auto &removed : prev_box.diff(new_box)) {
            remove_over(removed, id, false);
        }
        for (const auto &added : new_box.diff(prev_box)) {
            insert_over(value, added, id, false);
        }
        return *this;
    }

    RTree &insert_over(const Value &value, const Box<N> &box, const U64 id, const bool is_new) {
        bounds_ = bounds_ ? circumscribe(*bounds_, box) : box;
        Value &value_entry = values_[id];
        if (is_new) {
            value_entry = value;
        }
        const Ref<Value> value_ref = value_entry;
        for (auto [node, pos] : points_in(box)) {
            node->insert(pos, value_ref);
        }
        return *this;
    }

    RTree &remove_over(const Box<N> &box, const U64 id, const bool remove_all) {
        if (Value *value = values_.get(id)) {
            // TODO: Update bounds?
            const Ref<Value> value_ref = *value;
            for (auto [node, pos] : entries_in(box)) {
                node->remove(pos, value_ref);
            }
            if (remove_all) {
                values_.remove(id);
            }
            for (const U64 removed_id : garbage_) {
                nodes_.remove(removed_id);
            }
            garbage_.clear();
        }
        return *this;
    }

    const Options options_;
    const U64 grid_min_;
    Maybe<Box<N>> bounds_ = None;

    // Nodes keep references to the values stored in the above map to avoid storing two copies of each value.
    // These references are guaranteed stable as long as the value itself is not removed from the map.
    // See: https://cplusplus.com/reference/unordered_map/unordered_map/operator[]/
    Map<U64, Value> values_;

    Map<U64, Node> nodes_;
    Node *root_;
    U64 node_id_ = 0;

    // List of nodes to be removed when removal is complete
    List<U64> garbage_;
};

} // namespace nox
