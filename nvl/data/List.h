#pragma once

#include <vector>

#include "nvl/data/Range.h"
#include "nvl/macros/Implicit.h"
#include "nvl/macros/Pure.h"

namespace nvl {

template <typename Value>
class List : std::vector<Value> {
public:
    using parent = std::vector<Value>;
    using value_type = typename parent::value_type;
    using iterator = typename parent::iterator;
    using const_iterator = typename parent::const_iterator;
    using reverse_iterator = typename parent::reverse_iterator;
    using const_reverse_iterator = typename parent::const_reverse_iterator;

    using parent::parent;
    List() = default;

    template <typename Iterator>
        requires std::same_as<typename Iterator::value_type, Value>
    explicit List(const Range<Iterator> &range)
        : List<typename Range<Iterator>::value_type>(range.begin(), range.end()) {}

    pure Range<iterator> range() { return Range<iterator>(begin(), end()); }
    pure Range<const_iterator> range() const { return Range<const_iterator>(begin(), end()); }

    pure bool operator==(const List &rhs) const {
        return size() == rhs.size() && std::equal(begin(), end(), rhs.begin());
    }
    pure bool operator!=(const List &other) const { return !(*this == other); }

    using parent::begin;
    using parent::cbegin;
    using parent::cend;
    using parent::end;
    using parent::rbegin;
    using parent::rend;

    using parent::operator[];
    using parent::at;
    using parent::back;
    using parent::emplace_back;
    using parent::empty;
    using parent::front;
    using parent::pop_back;
    using parent::push_back;
    using parent::size;

    pure const Value *get_back() const { return empty() ? nullptr : &back(); }

    List<Value> &append(const List<Value> &rhs) {
        insert(end(), rhs.begin(), rhs.end());
        return *this;
    }

    List<Value> &remove(const Value &value) {
        auto it = std::remove(begin(), end(), value);
        parent::erase(it, end());
        return *this;
    }

    void clear() { parent::clear(); }

private:
    using std::vector<Value>::insert;
};

template <typename Value>
std::ostream &operator<<(std::ostream &os, const List<Value> &list) {
    return os << Range<typename List<Value>::const_iterator>(list.begin(), list.end());
}

} // namespace nvl