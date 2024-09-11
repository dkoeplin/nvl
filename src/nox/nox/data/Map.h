#pragma once

#include <unordered_map>

#include "nox/data/Maybe.h"
#include "nox/macros/Pure.h"

namespace nox {

template <typename K, typename V, typename Hash = std::hash<K>, typename Equal = std::equal_to<K>,
          typename Allocator = std::allocator<std::pair<const K, V>>>
class Map : std::unordered_map<K, V, Hash, Equal, Allocator> {
  public:
    using parent = std::unordered_map<K, V, Hash, Equal, Allocator>;
    using iterator = typename parent::iterator;
    using const_iterator = typename parent::const_iterator;

    using parent::operator[];
    using parent::at;
    using parent::end;
    using parent::size;

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

    pure bool is_empty() const { return parent::empty(); }

    struct const_unordered_range {
        explicit const_unordered_range(const Map &map) : parent_(map) {}
        pure const_iterator begin() const { return parent_.begin(); }
        pure const_iterator end() const { return parent_.end(); }
        const Map &parent_;
    };

    pure const_unordered_range unordered() const { return const_unordered_range(*this); }
};

template <typename K, typename V, typename Hash, typename Equal, typename Allocator>
inline std::ostream &operator<<(std::ostream &os, const Map<K, V, Hash, Equal, Allocator> &map) {
    os << "{";
    if (!map.is_empty()) {
        auto unordered = map.unordered();
        auto iter = unordered.begin();
        os << "{" << iter->first << ", ";
        os << iter->second << "}";
        for (++iter; iter != unordered.end(); ++iter) {
            os << ", {" << iter->first << ", " << iter->second << "}";
        }
    }
    return os << "}";
}

} // namespace nox
