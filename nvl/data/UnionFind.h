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
    UnionFind &add(const Item &a) {
        const U64 id_a = ids_.get_or(a, 0);
        if (id_a == 0) {
            ++count_;
            groups_[count_].insert(a);
            ids_[a] = count_;
        }
        return *this;
    }

    /// Marks elements `a` and `b` as equivalent, inserting them into a new set or combining their existing sets
    /// if either are already present.
    UnionFind &add(const Item &a, const Item &b) {
        const U64 id_a = ids_.get_or(a, 0);
        const U64 id_b = ids_.get_or(b, 0);
        return_if(id_a == id_b && id_a > 0, *this);
        U64 id = std::min(id_a, id_b);
        if (id == 0) {
            ++count_;
            id = count_;
        }
        auto &group = groups_[id];
        move_to_current_group(id, id_a, group, a);
        move_to_current_group(id, id_b, group, b);
        return *this;
    }

    pure bool has(const Item &item) const { return ids_.contains(item); }

    pure Range<Group> sets() const { return groups_.values(); }

private:
    void move_to_current_group(const U64 dst, const U64 src, Group &group, const Item &item) {
        return_if(src == dst);
        if (auto iter = groups_.find(src); iter != groups_.end()) {
            for (const Item &member : iter->second) {
                group.insert(member);
                ids_[member] = dst;
            }
            groups_.erase(src);
        } else {
            group.insert(item);
            ids_[item] = dst;
        }
    }

    U64 count_ = 0;
    Map<U64, Group> groups_;
    Map<Item, U64, Hash> ids_;
};

} // namespace nvl
