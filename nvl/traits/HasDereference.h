#pragma once

namespace nvl::traits {

/**
 * @concept HasDereferenceOperator
 * @brief Requires that type T can be dereferenced via the * operator.
 */
template <typename T>
concept HasDereferenceOperator = requires(T a) { *a; };

/**
 * @concept HasMemberAccessOperator
 * @brief Requires that type T can be dereferenced via the -> operator.
 */
template <typename T>
concept HasMemberAccessOperator = requires(T a) { a.operator->(); };

} // namespace nvl::traits
