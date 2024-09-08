#pragma once

#include <unordered_set>

namespace nox {

template <typename Value, typename Hash = std::hash<Value>> using Set = std::unordered_set<Value, Hash>;

} // namespace nox
