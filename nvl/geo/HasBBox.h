#pragma once

#include "nvl/macros/Aliases.h"

namespace nvl {

template <U64 N, typename T>
class Volume;

namespace trait {
/**
 * @concept HasBBox
 * @brief Requires type T to have a bbox() method which returns a bounding box by const reference.
 */
template <U64 N, typename T, typename Value>
concept HasBBox = requires(Value a) {
    { a.bbox() } -> std::convertible_to<Volume<N, T>>;
};

} // namespace trait

} // namespace nvl
