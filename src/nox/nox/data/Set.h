#pragma once

#include <unordered_set>

namespace nox {

template <typename Value, typename Hash = std::hash<Value>> using Set = std::unordered_set<Value, Hash>;

template <typename Value, typename Hash>
inline std::ostream &operator<<(std::ostream &os, const Set<Value, Hash> &set) {
    os << "{";
    if (!set.empty()) {
        auto iter = set.begin();
        os << *iter;
        for (++iter; iter != set.end(); ++iter) {
            os << ", " << *iter;
        }
    }
    return os << "}";
}

} // namespace nox

namespace std {

template <typename Value, typename Hash>
inline std::ostream &operator<<(std::ostream &os, const unordered_set<Value, Hash> &set) {
    os << "{";
    if (!set.empty()) {
        auto iter = set.begin();
        os << *iter;
        for (++iter; iter != set.end(); ++iter) {
            os << ", " << *iter;
        }
    }
    return os << "}";
}


} // namespace std
