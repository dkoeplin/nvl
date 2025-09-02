#pragma once

#include "nvl/data/Map.h"
#include "nvl/data/Range.h"
#include "nvl/data/Set.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/Pure.h"

namespace nvl {

/**
 * @class UnionFind
 * @brief Data structure which organizes items into "equivalent" groups.
 *
 * Items are added in pairs, where adding two items together marks them as being in the same group.
 *
 * @tparam Item The item type being stored.
 * @tparam Hash The hash function used for items in each set.
 */
template <typename Item, typename Hash = std::hash<Item>>
class UnionFind {
public:
    using Group = Set<Item, Hash>;

    /// Inserts a single element into its own set.
    U64 add(const Item &a) {
        const U64 id_a = ids_.get_or(a, 0);
        return_if(id_a != 0, id_a);
        ++count_;
        groups_[count_].insert(a);
        make(count_);
        return ids_[a] = count_;
    }

    /// Marks elements `a` and `b` as equivalent, inserting them into a new set or combining their existing sets
    /// if either are already present.
    UnionFind &add(const Item &a, const Item &b) {
        merge(add(a), add(b));
        return *this;
    }

    pure bool has(const Item &item) const { return ids_.contains(item); }

    pure Range<Group> sets() const {
        update_groups();
        return groups_.values();
    }

private:
    void update_groups() const {
        groups_.clear();
        for (const auto &[item, id] : ids_) {
            const U64 set = find(id);
            groups_[set].insert(item);
        }
    }

    // Internals adapted from https://cp-algorithms.com/data_structures/disjoint_set_union.html
    U64 find(const U64 v) const {
        U64 value = v;
        while (value != parent_.at(value)) {
            value = parent_.at(value);
        }
        // "Path compression" - memoize the root to avoid traversing the whole path each time
        parent_[v] = value;
        return value;
    }

    void make(const U64 v) {
        parent_[v] = v;
        size_[v] = 1;
    }

    void merge(U64 a, U64 b) {
        a = find(a);
        b = find(b);
        if (a != b) {
            // Union by size
            if (size_[a] < size_[b])
                std::swap(a, b);
            parent_[b] = a;
            size_[a] += size_[b];
        }
    }

    U64 count_ = 0;
    mutable Map<U64, Group> groups_;
    Map<Item, U64, Hash> ids_;
    mutable Map<U64, U64> parent_;
    Map<U64, U64> size_;
};

} // namespace nvl
