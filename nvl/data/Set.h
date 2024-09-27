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

    struct iterator final : AbstractIterator<Value> {
        class_tag(Set::iterator, AbstractIterator<Value>);
        template <View Type = View::kImmutable>
        static Iterator<Value, Type> begin(const Set &set) {
            return make_iterator<iterator, Type>(set._begin());
        }
        template <View Type = View::kImmutable>
        static Iterator<Value, Type> end(const Set &set) {
            return make_iterator<iterator, Type>(set._end());
        }
        explicit iterator(typename parent::const_iterator iter) : iter(iter) {}
        pure std::unique_ptr<AbstractIterator<Value>> copy() const override {
            return std::make_unique<iterator>(*this);
        }
        void increment() override { ++iter; }
        pure const Value *ptr() override { return &*iter; }
        pure bool equals(const AbstractIterator<Value> &rhs) const override {
            auto *b = dyn_cast<iterator>(&rhs);
            return b && iter == b->iter;
        }

        typename parent::const_iterator iter;
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

    struct Unordered {
        explicit Unordered(Set &set) : set(set) {}
        pure Iterator<Value, View::kMutable> begin() { return iterator::template begin<View::kMutable>(set); }
        pure Iterator<Value, View::kMutable> end() { return iterator::template end<View::kMutable>(set); }
        pure Iterator<Value> begin() const { return iterator::template begin(set); }
        pure Iterator<Value> end() const { return iterator::template end(set); }

        pure Range<Value, View::kMutable> values() { return {begin(), end()}; }
        pure Range<Value> values() const { return {begin(), end()}; }

        Set &set;
    } unordered = Unordered(*this);

    pure bool operator==(const Set &rhs) const {
        return_if(size() != rhs.size(), false);
        const Iterator<Value> rhs_end = rhs.unordered.end();
        for (Iterator<Value> iter = unordered.begin(); iter != unordered.end(); ++iter) {
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
    return os << set.unordered.values();
}

} // namespace nvl
