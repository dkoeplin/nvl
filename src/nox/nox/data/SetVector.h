#pragma once

#include <list>
#include <unordered_map>

#include "nox/data/Maybe.h"
#include "nox/macros/Aliases.h"
#include "nox/macros/Pure.h"

namespace nox {

template <typename Value, typename Hash = std::hash<Value>> // Value / Hash
class SetVector {
  public:
    using Vec = std::vector<Value>;

    using value_type = typename Vec::value_type;
    using iterator = typename Vec::iterator;
    using const_iterator = typename Vec::const_iterator;
    using reverse_iterator = typename Vec::reverse_iterator;
    using const_reverse_iterator = typename Vec::const_reverse_iterator;

    SetVector() = default;
    SetVector(std::initializer_list<Value> init) {
        map_.reserve(init.size());
        vec_.reserve(init.size());
        for (const Value &v : init) {
            if (auto iter = map_.find(v); iter == map_.end()) {
                map_[v] = vec_.size();
                vec_.push_back(v);
            }
        }
    }

    iterator begin() { return vec_.begin(); }
    iterator end() { return vec_.end(); }
    pure const_iterator begin() const { return vec_.begin(); }
    pure const_iterator end() const { return vec_.end(); }

    pure U64 size() const { return vec_.size(); }
    pure bool is_empty() const { return vec_.empty(); }

    Value &operator[](U64 i) { return vec_[i]; }
    pure const Value &operator[](U64 i) const { return vec_[i]; }

    Maybe<Value &> get(U64 i) const {
        if (i < vec_.size()) {
            return Some(vec_.at(i));
        }
        return None;
    }

    const Value &get_or(U64 i, const Value &v) const {
        if (i < vec_.size()) {
            return vec_.at(i);
        }
        return v;
    }

    SetVector &append(const Value &v) {
        if (!map_.count(v)) {
            map_[v] = vec_.size();
            vec_.push_back(v);
        }
        return *this;
    }

    SetVector &prepend(const Value &v) {
        if (!map_.count(v)) {
            map_[v] = 0;
            vec_.insert(vec_.begin(), v);
        }
        return *this;
    }

  private:
    std::unordered_map<Value, U64, Hash> map_;
    std::vector<Value> vec_;
    Hash hash_;
};

} // namespace nox
