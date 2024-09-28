#pragma once

namespace nvl::trait {

/**
 * @concept HasEquality
 * @brief Requires that instances of type T can be compared via the == operator.
 */
template <typename T>
concept HasEquality = requires(T a) { a.operator==(); };

} // namespace nvl::trait
