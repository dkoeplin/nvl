#pragma once

#include <vector>

#include "nvl/data/Maybe.h"
#include "nvl/data/Ref.h"
#include "nvl/macros/Pure.h"

namespace nvl {

template <typename Value>
class List : std::vector<Value> {
  public:
    using parent = std::vector<Value>;
    using value_type = typename std::vector<Value>::value_type;
    using iterator = typename std::vector<Value>::iterator;
    using const_iterator = typename std::vector<Value>::const_iterator;
    using reverse_iterator = typename std::vector<Value>::reverse_iterator;
    using const_reverse_iterator = typename std::vector<Value>::const_reverse_iterator;

    using parent::parent;
    List() = default;

    pure bool operator==(const List &other) const {
        return size() == other.size() && std::equal(begin(), end(), other.begin());
    }
    pure bool operator!=(const List &other) const { return !(*this == other); }

    using std::vector<Value>::begin;
    using std::vector<Value>::end;
    using std::vector<Value>::rbegin;
    using std::vector<Value>::rend;
    using std::vector<Value>::cbegin;
    using std::vector<Value>::cend;

    using std::vector<Value>::operator[];
    using std::vector<Value>::at;
    using std::vector<Value>::back;
    using std::vector<Value>::emplace_back;
    using std::vector<Value>::empty;
    using std::vector<Value>::front;
    using std::vector<Value>::push_back;
    using std::vector<Value>::pop_back;
    using std::vector<Value>::size;

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
    os << "{";
    auto iter = list.begin();
    if (iter != list.end()) {
        os << *iter;
    }
    for (; iter != list.end(); ++iter) {
        os << ", " << *iter;
    }
    return os << "}";
}

} // namespace nvl