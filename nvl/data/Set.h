#pragma once

#include <unordered_set>

#include "nvl/data/Range.h"
#include "nvl/macros/ReturnIf.h"

namespace nvl {

template <typename Value, typename Hash = std::hash<Value>>
class Set : std::unordered_set<Value, Hash> {
public:
    using parent = std::unordered_set<Value, Hash>;
    using value_type = typename parent::value_type;
    using iterator = typename parent::iterator;
    using const_iterator = typename parent::const_iterator;

    using parent::parent;

    template <typename Iterator>
        requires std::same_as<typename Iterator::value_type, Value>
    explicit Set(const Range<Iterator> &range)
        : Set<typename Range<Iterator>::value_type>(range.begin(), range.end()) {}

    using parent::begin;
    using parent::end;

    using parent::contains;
    using parent::emplace;
    using parent::empty;
    using parent::find;
    using parent::insert;
    using parent::size;

    template <typename Iterator>
        requires std::same_as<typename Iterator::value_type, Value>
    Set &insert(const Range<Iterator> &range) {
        parent::insert(range.begin(), range.end());
        return *this;
    }

    pure Range<iterator> unordered() { return Range(begin(), end()); }
    pure Range<const_iterator> unordered() const { return Range(begin(), end()); }

    pure bool operator==(const Set &rhs) const {
        return_if(size() != rhs.size(), false);
        const auto rhs_end = rhs.end();
        for (auto iter = begin(); iter != end(); ++iter) {
            auto rhs_iter = rhs.find(*iter);
            return_if(rhs_iter == rhs_end || *iter != *rhs_iter, false);
        }
        return true;
    }
    pure bool operator!=(const Set &rhs) const { return !(*this == rhs); }
};

template <typename Value, typename Hash>
std::ostream &operator<<(std::ostream &os, const Set<Value, Hash> &set) {
    return os << Range<typename Set<Value, Hash>::const_iterator>(set.begin(), set.end());
}

} // namespace nvl
