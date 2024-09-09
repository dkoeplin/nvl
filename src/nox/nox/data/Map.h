#pragma once

#include <unordered_map>

#include "nox/data/Maybe.h"
#include "nox/macros/Aliases.h"
#include "nox/macros/Pure.h"

namespace nox {

template <typename K, typename V, typename Hash = std::hash<K>> class Map {
  public:
    using iterator = typename std::unordered_map<K, V, Hash>::iterator;
    using const_iterator = typename std::unordered_map<K, V, Hash>::const_iterator;

    Map() = default;
    Map(std::initializer_list<std::pair<K, V>> values) {
        for (const auto &value : values) {
            map.insert(value);
        }
    }

    pure const V &operator[](const K &key) const { return map.at(key); }
    V &operator[](const K &key) { return map[key]; }

    pure V *get(const K &key) const {
        if (auto iter = map.find(key); iter != map.end()) {
            // TODO: Why does this require casting?
            return const_cast<V *>(&iter->second);
        }
        return nullptr;
    }

    pure const V &get_or(const K &key, const V &v) const {
        auto iter = map.find(key);
        return iter != map.end() ? iter->second : v;
    }

    V &get_or_add(const K &key, V v) {
        if (auto iter = map.find(key); iter != map.end()) {
            return iter->second;
        }
        return map[key] = std::move(v);
    }

    V &get_or_lazily_add(const K &key, const std::function<V()> &func) {
        if (auto iter = map.find(key); iter != map.end()) {
            return iter->second;
        }
        return map[key] = std::move(func());
    }

    void remove(const K &key) { map.erase(key); }

    pure bool has(const K &key) const { return map.find(key) != map.end(); }

    pure U64 size() const { return map.size(); }

    pure bool is_empty() const { return map.empty(); }

    struct const_unordered_range {
        explicit const_unordered_range(const Map &map) : parent_(map) {}
        pure const_iterator begin() const { return parent_.map.begin(); }
        pure const_iterator end() const { return parent_.map.end(); }
        const Map &parent_;
    };

    pure bool operator==(const Map &other) const { return map == other.map; }
    pure bool operator!=(const Map &other) const { return map != other.map; }

    pure const_unordered_range unordered() const { return const_unordered_range(*this); }

  private:
    std::unordered_map<K, V, Hash> map;
};

template <typename K, typename V, typename Hash>
inline std::ostream &operator<<(std::ostream &os, const Map<K, V, Hash> &map) {
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
