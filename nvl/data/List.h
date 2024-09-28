#pragma once

#include <vector>

#include "nvl/data/Iterator.h"
#include "nvl/data/Range.h"
#include "nvl/macros/Pure.h"

namespace nvl {

template <typename Value>
class List : std::vector<Value> {
public:
    using parent = std::vector<Value>;
    using value_type = typename parent::value_type;

    struct iterator final : AbstractIteratorCRTP<iterator, Value>, parent::const_iterator {
        class_tag(List::iterator, AbstractIterator<Value>);
        template <View Type = View::kImmutable>
        static Iterator<Value, Type> begin(const List &list) {
            return make_iterator<iterator, Type>(list._begin());
        }
        template <View Type = View::kImmutable>
        static Iterator<Value, Type> end(const List &list) {
            return make_iterator<iterator, Type>(list._end());
        }
        explicit iterator(typename parent::const_iterator iter) : parent::const_iterator(iter) {}
        void increment() override { parent::const_iterator::operator++(); }
        pure const Value *ptr() override { return &parent::const_iterator::operator*(); }
        pure bool operator==(const iterator &rhs) const override { return this->base() == rhs.base(); }
    };

    using parent::parent;
    List() = default;

    explicit List(Range<Value> range) : List(range.begin(), range.end()) {}

    pure bool operator==(const List &rhs) const {
        return size() == rhs.size() && std::equal(begin(), end(), rhs.begin());
    }
    pure bool operator!=(const List &other) const { return !(*this == other); }

    using parent::operator[];
    using parent::at;
    using parent::back;
    using parent::emplace_back;
    using parent::empty;
    using parent::front;
    using parent::pop_back;
    using parent::push_back;
    using parent::size;

    pure Iterator<Value, View::kMutable> begin() { return iterator::template begin<View::kMutable>(*this); }
    pure Iterator<Value, View::kMutable> end() { return iterator::template end<View::kMutable>(*this); }
    pure Iterator<Value> begin() const { return iterator::template begin<View::kImmutable>(*this); }
    pure Iterator<Value> end() const { return iterator::template end<View::kImmutable>(*this); }

    pure Range<Value, View::kMutable> range() { return {begin(), end()}; }
    pure Range<Value> range() const { return {begin(), end()}; }

    pure const Value *get_back() const { return empty() ? nullptr : &back(); }

    List<Value> &append(const List<Value> &rhs) {
        insert(end(), rhs.begin(), rhs.end());
        return *this;
    }

    List<Value> &remove(const Value &value) {
        auto it = std::remove(parent::begin(), parent::end(), value);
        parent::erase(it, parent::end());
        return *this;
    }

    void clear() { parent::clear(); }

private:
    friend struct iterator;
    typename std::vector<Value>::const_iterator _begin() const { return parent::begin(); }
    typename std::vector<Value>::const_iterator _end() const { return parent::end(); }

    using std::vector<Value>::insert;
};

template <typename Value>
std::ostream &operator<<(std::ostream &os, const List<Value> &list) {
    return os << list.range();
}

} // namespace nvl