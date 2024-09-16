#pragma once

namespace nvl::traits {

/**
 * @concept HasBBox
 * @brief Requires type T to have a bbox() method which returns a bounding box.
 */
template <typename T>
concept HasBBox = requires(T a) { a.bbox(); };

} // namespace nvl::traits
