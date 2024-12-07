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
        pure U64 operator-(const iterator &rhs) const override { return this->base() - rhs.base(); }
        pure bool operator==(const iterator &rhs) const override { return this->base() == rhs.base(); }
    };

    struct reverse_iterator final : AbstractIteratorCRTP<reverse_iterator, Value>, parent::const_reverse_iterator {
        class_tag(List::reverse_iterator, AbstractIterator<Value>);
        template <View Type = View::kImmutable>
        static Iterator<Value, Type> begin(const List &list) {
            return make_iterator<reverse_iterator, Type>(list._rbegin());
        }
        template <View Type = View::kImmutable>
        static Iterator<Value, Type> end(const List &list) {
            return make_iterator<reverse_iterator, Type>(list._rend());
        }
        explicit reverse_iterator(typename parent::const_reverse_iterator iter)
            : parent::const_reverse_iterator(iter) {}
        void increment() override { parent::const_reverse_iterator::operator++(); }
        pure const Value *ptr() override { return &parent::const_reverse_iterator::operator*(); }
        pure U64 operator-(const reverse_iterator &rhs) const override { return this->base() - rhs.base(); }
        pure bool operator==(const reverse_iterator &rhs) const override { return this->base() == rhs.base(); }
    };

    using parent::parent;
    List() = default;

    explicit List(Range<Value> range) : List(range.begin(), range.end()) {}

    pure bool operator==(const List &rhs) const {
        // HOT SPOT: Using the custom iterators slows comparison down by ~20x. Likely to do with std::__unwrap_iter?
        return size() == rhs.size() && std::equal(_begin(), _end(), rhs._begin());
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

    pure MIterator<Value> begin() { return iterator::template begin<View::kMutable>(*this); }
    pure MIterator<Value> end() { return iterator::template end<View::kMutable>(*this); }
    pure Iterator<Value> begin() const { return iterator::template begin<View::kImmutable>(*this); }
    pure Iterator<Value> end() const { return iterator::template end<View::kImmutable>(*this); }

    pure MIterator<Value> rbegin() { return reverse_iterator::template begin<View::kMutable>(*this); }
    pure MIterator<Value> rend() { return reverse_iterator::template end<View::kMutable>(*this); }
    pure Iterator<Value> rbegin() const { return reverse_iterator::template begin<View::kImmutable>(*this); }
    pure Iterator<Value> rend() const { return reverse_iterator::template end<View::kImmutable>(*this); }

    pure MRange<Value> range() { return {begin(), end()}; }
    pure Range<Value> range() const { return {begin(), end()}; }

    pure MRange<Value> rrange() { return {rbegin(), rend()}; }
    pure Range<Value> rrange() const { return {rbegin(), rend()}; }

    pure const Value *get_back() const { return empty() ? nullptr : &back(); }

    List<Value> &append(const List<Value> &rhs) { return append(rhs.range()); }
    List<Value> &append(const Range<Value> &rhs) {
        for (const Value &x : rhs) {
            push_back(x);
        }
        return *this;
    }

    List<Value> &remove(const Value &value) {
        auto it = std::remove(parent::begin(), parent::end(), value);
        parent::erase(it, parent::end());
        return *this;
    }

    List<Value> &remove_if(const std::function<bool(Value)> &cond) {
        auto it = std::remove_if(parent::begin(), parent::end(), cond);
        parent::erase(it, parent::end());
        return *this;
    }

    void clear() { parent::clear(); }

private:
    friend struct iterator;
    typename std::vector<Value>::const_iterator _begin() const { return parent::begin(); }
    typename std::vector<Value>::const_iterator _end() const { return parent::end(); }
    typename std::vector<Value>::const_reverse_iterator _rbegin() const { return parent::rbegin(); }
    typename std::vector<Value>::const_reverse_iterator _rend() const { return parent::rend(); }

    using std::vector<Value>::insert;
};

template <typename Value>
std::ostream &operator<<(std::ostream &os, const List<Value> &list) {
    return os << list.range();
}

} // namespace nvl