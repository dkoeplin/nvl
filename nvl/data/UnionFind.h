#pragma once

#include "nvl/data/List.h"
#include "nvl/data/Map.h"
#include "nvl/data/Range.h"
#include "nvl/data/Ref.h"
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
        ++count_;
        ids_[a] = count_;
        groups_[count_].insert(a);
        return *this;
    }

    /// Marks elements `a` and `b` as equivalent, inserting them into a new set or combining their existing sets
    /// if either are already present.
    UnionFind &add(const Item &a, const Item &b) {
        U64 &id_a = ids_.get_or_add(a, 0);
        U64 &id_b = ids_.get_or_add(b, 0);
        return_if(id_a == id_b && id_a > 0, *this);
        U64 id = 0;
        if (id_a > 0 || id_b > 0) {
            id = std::max(id_a, id_b);
        } else {
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
    void move_to_current_group(U64 dst, U64 &src, Group &group, const Item &item) {
        if (src != dst) {
            if (auto iter = groups_.find(src); iter != groups_.end()) {
                const auto &set = iter->second;
                group.insert(set.values());
                groups_.erase(iter);
            } else {
                group.insert(item);
            }
            src = dst;
        }
    }

    U64 count_ = 0;
    Map<U64, Group> groups_;
    Map<Item, U64, Hash> ids_;
};

} // namespace nvl
