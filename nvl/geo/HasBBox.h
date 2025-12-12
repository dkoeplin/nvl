#pragma once

#include <concepts>

#include "nvl/macros/Aliases.h"
#include "nvl/macros/Expand.h"

namespace nvl {

template <U64 N, typename T>
class Volume;

namespace detail {

template <U64 N, typename T, typename Value>
concept HasBBoxDirect = requires(Value a) {
    { a.bbox() } -> std::convertible_to<Volume<N, T>>;
};

template <U64 N, typename T, typename Value>
concept HasBBoxWrapped = requires(Value a) {
    { a->bbox() } -> std::convertible_to<Volume<N, T>>;
};

} // namespace detail

namespace trait {
/**
 * @concept HasBBox
 * @brief Requires type T to have a bbox() method which returns a bounding box by const reference.
 */
template <U64 N, typename T, typename Value>
concept HasBBox = detail::HasBBoxDirect<N, T, Value> || detail::HasBBoxWrapped<N, T, Value>;
} // namespace trait

/**
 * Returns the bounding box for a value-like or pointer-like instance.
 */
template <U64 N, typename T, typename Value>
    requires trait::HasBBox<N, T, Value>
expand Volume<N, T> bbox(const Value &value) {
    if constexpr (detail::HasBBoxDirect<N, T, Value>) {
        return value.bbox();
    } else {
        return value->bbox();
    }
}

} // namespace nvl
