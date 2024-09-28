#pragma once

#include <unordered_set>

#include "nvl/data/Iterator.h"
#include "nvl/data/Range.h"
#include "nvl/macros/ReturnIf.h"

namespace nvl {

template <typename Value, typename Hash = std::hash<Value>>
class Set final : std::unordered_set<Value, Hash> {
public:
    using parent = std::unordered_set<Value, Hash>;
    using value_type = typename parent::value_type;

    struct iterator final : AbstractIteratorCRTP<iterator, Value>, parent::const_iterator {
        class_tag(Set::iterator, AbstractIterator<Value>);
        template <View Type = View::kImmutable>
        static Iterator<Value, Type> begin(const Set &set) {
            return make_iterator<iterator, Type>(set._begin());
        }
        template <View Type = View::kImmutable>
        static Iterator<Value, Type> end(const Set &set) {
            return make_iterator<iterator, Type>(set._end());
        }
        explicit iterator(typename parent::const_iterator iter) : parent::const_iterator(iter) {}
        void increment() override { parent::const_iterator::operator++(); }
        pure const Value *ptr() override { return &parent::const_iterator::operator*(); }
        pure bool operator==(const iterator &rhs) const override {
            return *static_cast<const typename parent::const_iterator *>(this) == rhs;
        }
    };

    explicit Set(const Range<Value> &range) : Set(range.begin(), range.end()) {}

    Set(const Set &rhs) : parent(rhs) {}

    using parent::emplace;
    using parent::empty;
    using parent::insert;
    using parent::parent;
    using parent::size;

    Set &operator=(const Set &rhs) {
        parent::operator==(rhs);
        return *this;
    }

    Set &insert(const Range<Value> &range) {
        parent::insert(range.begin(), range.end());
        return *this;
    }

    Set &remove(const Value &value) {
        parent::erase(value);
        return *this;
    }

    Set &remove(const Range<Value> &range) {
        for (auto &value : range) {
            parent::erase(value);
        }
        return *this;
    }

    pure Iterator<Value> find(const Value &value) const {
        typename parent::const_iterator iter = parent::find(value);
        return make_iterator<iterator>(iter);
    }

    pure bool has(const Value &value) const { return parent::contains(value); }

    pure Iterator<Value, View::kMutable> begin() { return iterator::template begin<View::kMutable>(*this); }
    pure Iterator<Value, View::kMutable> end() { return iterator::template end<View::kMutable>(*this); }
    pure Iterator<Value> begin() const { return iterator::template begin(*this); }
    pure Iterator<Value> end() const { return iterator::template end(*this); }

    pure Range<Value, View::kMutable> values() { return {begin(), end()}; }
    pure Range<Value> values() const { return {begin(), end()}; }

    pure bool operator==(const Set &rhs) const {
        return_if(size() != rhs.size(), false);
        const Iterator<Value> rhs_end = rhs.end();
        for (Iterator<Value> iter = begin(); iter != end(); ++iter) {
            auto rhs_iter = rhs.find(*iter);
            return_if(rhs_iter == rhs_end || *iter != *rhs_iter, false);
        }
        return true;
    }
    pure bool operator!=(const Set &rhs) const { return !(*this == rhs); }

protected:
    pure typename parent::const_iterator _begin() const { return parent::begin(); }
    pure typename parent::const_iterator _end() const { return parent::end(); }
};

template <typename Value, typename Hash>
std::ostream &operator<<(std::ostream &os, const Set<Value, Hash> &set) {
    return os << set.values();
}

} // namespace nvl
