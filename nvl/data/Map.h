#pragma once

#include <unordered_map>

#include "nvl/data/Range.h"
#include "nvl/macros/Pure.h"

namespace nvl {

template <typename K, typename V, typename Hash = std::hash<K>, typename Equal = std::equal_to<K>,
          typename Allocator = std::allocator<std::pair<const K, V>>>
class Map : std::unordered_map<K, V, Hash, Equal, Allocator> {
public:
    using parent = std::unordered_map<K, V, Hash, Equal, Allocator>;
    using iterator = typename parent::iterator;
    using const_iterator = typename parent::const_iterator;

    using parent::operator[];
    using parent::at;
    using parent::clear;
    using parent::empty;
    using parent::end;
    using parent::size;

    class value_iterator {
    public:
        using value_type = V;
        using pointer = V *;
        using reference = V &;

        explicit value_iterator(typename Map<K, V>::iterator iter) : iter_(iter) {}

        value_iterator &operator++() {
            ++iter_;
            return *this;
        }
        V &operator*() { return iter_->second; }
        V *operator->() { return &iter_->second; }

        pure bool operator==(const value_iterator &rhs) const { return iter_ == rhs.iter_; }
        pure bool operator!=(const value_iterator &rhs) const { return iter_ != rhs.iter_; }

    private:
        typename Map<K, V>::iterator iter_;
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
    pure V &emplace(Args &&...args) {
        auto [iter, _] = parent::emplace(std::forward<Args>(args)...);
        return iter->second;
    }

    pure V *get(const K &key) const {
        if (auto iter = parent::find(key); iter != end()) {
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
        if (auto iter = parent::find(key); iter != end()) {
            return iter->second;
        }
        return parent::emplace(key, v).first->second;
    }

    V &get_or_lazily_add(const K &key, const std::function<V()> &func) {
        if (auto iter = parent::find(key); iter != end()) {
            return iter->second;
        }
        return parent::emplace(key, func()).first->second;
    }

    void remove(const K &key) { parent::erase(key); }

    pure bool has(const K &key) const { return parent::contains(key); }

    pure bool operator==(const Map &other) const { return std::operator==(*this, other); }
    pure bool operator!=(const Map &other) const { return std::operator!=(*this, other); }

    pure Range<const_iterator> unordered_entries() const { return {parent::begin(), parent::end()}; }
    pure Range<value_iterator> unordered_values() {
        return {value_iterator(parent::begin()), value_iterator(parent::end())};
    }
};

template <typename K, typename V, typename Hash, typename Equal, typename Allocator>
std::ostream &operator<<(std::ostream &os, const Map<K, V, Hash, Equal, Allocator> &map) {
    os << "{";
    auto range = map.unordered_entries().once();
    if (!range.empty()) {
        os << range->first << ": " << range->second;
    }
    while (range.has_next()) {
        os << ", " << range->first << ": " << range->second;
    }
    return os << "}";
}

} // namespace nvl
