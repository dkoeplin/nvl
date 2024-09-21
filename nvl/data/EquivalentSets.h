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
 * @class EquivalentSets
 * @brief Data structure which organizes items into "equivalent" groups.
 *
 * Items are added in pairs, where adding two items together marks them as being in the same group.
 *
 * @tparam Item The item type being stored.
 * @tparam Hash The hash function used for items in each set.
 */
template <typename Item, typename Hash = std::hash<Item>>
class EquivalentSets {
public:
    using Group = Set<Item, Hash>;

    EquivalentSets &add(const Item &a, const Item &b) {
        U64 &id_a = ids_[a];
        U64 &id_b = ids_[b];
        return_if(id_a == id_b && id_a > 0, *this);
        count_ = id_a > 0 ? id_a : id_b > 0 ? id_b : count_ + 1;
        auto &group = groups_[count_];
        move_to_current_group(id_a, group, a);
        move_to_current_group(id_b, group, b);
        return *this;
    }

    pure bool has(const Item &item) const { return ids_.contains(item); }

    /*template <typename Iterator>
        requires std::is_same_v<typename Iterator::value_type, Item>
    EquivalentSets &add(const Range<Iterator> &range) {}*/

    using set_iterator = typename Map<U64, Group>::value_iterator;
    Range<set_iterator> sets() { return groups_.unordered_values(); }

private:
    void move_to_current_group(U64 &id, Group &group, const Item &item) {
        if (id != count_) {
            if (auto iter = groups_.find(id); iter != groups_.end()) {
                group.insert(iter->second.begin(), iter->second.end());
                groups_.erase(iter);
            } else {
                group.insert(item);
            }
            id = count_;
        }
    }

    U64 count_ = 0;
    Map<U64, Group> groups_;
    Map<Item, U64, Hash> ids_;
};

} // namespace nvl
