#pragma once

#include <unordered_map>

#include "nvl/data/Iterator.h"
#include "nvl/data/Range.h"
#include "nvl/macros/Pure.h"

namespace nvl {

template <typename K, typename V, typename Hash = std::hash<K>, typename Equal = std::equal_to<K>,
          typename Allocator = std::allocator<std::pair<const K, V>>>
class Map : std::unordered_map<K, V, Hash, Equal, Allocator> {
public:
    using parent = std::unordered_map<K, V, Hash, Equal, Allocator>;
    using Entry = std::pair<const K, V>;

    using parent::operator[];
    using parent::at;
    using parent::clear;
    using parent::empty;
    using parent::end;
    using parent::size;

    struct entry_iterator final : AbstractIteratorCRTP<entry_iterator, Entry>, parent::const_iterator {
        class_tag(Map::entry_iterator, AbstractIterator<Entry>);
        template <View Type = View::kImmutable>
        static Iterator<Entry, Type> begin(const Map &map) {
            return make_iterator<entry_iterator, Type>(map._begin());
        }
        template <View Type = View::kImmutable>
        static Iterator<Entry, Type> end(const Map &map) {
            return make_iterator<entry_iterator, Type>(map._end());
        }

        explicit entry_iterator(typename parent::const_iterator iter) : parent::const_iterator(iter) {}

        void increment() override { parent::const_iterator::operator++(); }
        const Entry *ptr() override { return &parent::const_iterator::operator*(); }

        pure bool operator==(const entry_iterator &rhs) const override {
            return *static_cast<const typename parent::const_iterator *>(this) == rhs;
        }
    };

    struct viterator final : AbstractIteratorCRTP<viterator, V>, parent::const_iterator {
        class_tag(Map::viterator, AbstractIterator<V>);
        using value_type = V;

        template <View Type = View::kImmutable>
        static Iterator<V, Type> begin(const Map &map) {
            return make_iterator<viterator, Type>(map._begin());
        }
        template <View Type = View::kImmutable>
        static Iterator<V, Type> end(const Map &map) {
            return make_iterator<viterator, Type>(map._end());
        }

        explicit viterator(typename parent::const_iterator iter) : parent::const_iterator(iter) {}

        void increment() override { parent::const_iterator::operator++(); }
        const V *ptr() override { return &parent::const_iterator::operator->()->second; }

        pure bool operator==(const viterator &rhs) const override {
            return *static_cast<const typename parent::const_iterator *>(this) == rhs;
        }
    };

    Map() : parent() {}
    Map(std::initializer_list<std::pair<const K, V>> init) : parent(init) {}

    // The copy constructor and copy assignment operator are deleted if the K or value are not copyable
    Map(const Map &rhs)
        requires std::is_copy_constructible_v<K> && std::is_copy_constructible_v<V> && std::is_copy_assignable_v<K> &&
                 std::is_copy_assignable_v<V>
        : parent(rhs) {}
    Map &operator=(const Map &rhs)
        requires std::is_copy_constructible_v<V> && std::is_copy_constructible_v<V> && std::is_copy_assignable_v<K> &&
                 std::is_copy_assignable_v<V>
    {
        parent::operator=(rhs);
        return *this;
    }

    template <typename... Args>
    V &emplace(Args &&...args) {
        auto [iter, _] = parent::emplace(std::forward<Args>(args)...);
        return iter->second;
    }

    template <typename... Args>
    V &try_emplace(const K &key, Args &&...args) {
        auto [iter, _] = parent::try_emplace(key, std::forward<Args>(args)...);
        return iter->second;
    }

    Map &erase(Iterator<Entry> iter) {
        if (auto *entry_iter = iter.template dyn_cast<entry_iterator>()) {
            parent::erase(*entry_iter);
        }
        return *this;
    }

    Map &erase(const K &key) {
        parent::erase(key);
        return *this;
    }

    pure Iterator<Entry> find(const K &key) const {
        typename parent::const_iterator iter = parent::find(key);
        return make_iterator<entry_iterator>(iter);
    }
    pure MIterator<Entry> find(const K &key) {
        typename parent::const_iterator iter = parent::find(key);
        return make_miterator<entry_iterator>(iter);
    }

    pure V *get(const K &key) const {
        if (auto iter = parent::find(key); parent::end() != iter) {
            // TODO: Why does this require casting?
            return const_cast<V *>(&iter->second);
        }
        return nullptr;
    }

    pure const V &get_or(const K &key, const V &v) const {
        auto iter = find(key);
        return iter != end() ? iter->second : v;
    }

    V &get_or_add(const K &key, V v) {
        if (auto iter = find(key); iter != end()) {
            return iter->second;
        }
        return parent::emplace(key, v).first->second;
    }

    V &get_or_lazily_add(const K &key, const std::function<V()> &func) {
        if (auto iter = find(key); iter != end()) {
            return iter->second;
        }
        return parent::emplace(key, func()).first->second;
    }

    void remove(const K &key) { parent::erase(key); }
    void remove(const Range<K> &keys) {
        for (const K &key : keys) {
            parent::erase(key);
        }
    }

    pure bool has(const K &key) const { return parent::contains(key); }

    pure bool operator==(const Map &other) const { return std::operator==(*this, other); }
    pure bool operator!=(const Map &other) const { return std::operator!=(*this, other); }

    pure MRange<Entry> entries() { return {begin(), end()}; }
    pure Range<Entry> entries() const { return {begin(), end()}; }

    pure MIterator<Entry> begin() { return entry_iterator::template begin<View::kMutable>(*this); }
    pure MIterator<Entry> end() { return entry_iterator::template end<View::kMutable>(*this); }
    pure Iterator<Entry> begin() const { return entry_iterator::template begin(*this); }
    pure Iterator<Entry> end() const { return entry_iterator::template end(*this); }

    pure MRange<V> values() { return {values_begin(), values_end()}; }
    pure Range<V> values() const { return {values_begin(), values_end()}; }

    pure MIterator<V> values_begin() { return viterator::template begin<View::kMutable>(*this); }
    pure MIterator<V> values_end() { return viterator::template end<View::kMutable>(*this); }
    pure Iterator<V> values_begin() const { return viterator::template begin(*this); }
    pure Iterator<V> values_end() const { return viterator::template end(*this); }

protected:
    pure typename parent::const_iterator _begin() const { return parent::begin(); }
    pure typename parent::const_iterator _end() const { return parent::end(); }
};

template <typename K, typename V, typename Hash, typename Equal, typename Allocator>
std::ostream &operator<<(std::ostream &os, const Map<K, V, Hash, Equal, Allocator> &map) {
    os << "{";
    auto once = map.entries().once();
    if (!once.empty()) {
        os << once->first << ": " << once->second;
        ++once;
    }
    while (once.has_next()) {
        os << ", " << once->first << ": " << once->second;
        ++once;
    }
    return os << "}";
}

} // namespace nvl
