#pragma once

namespace nvl::traits {

/**
 * @concept HasID
 * @brief Requires type T to have an .id() method which returns an unsigned integer unique to the instance.
 */
template <typename T>
concept HasID = requires(T a) { a.id(); };

} // namespace nvl::traits
