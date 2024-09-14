#pragma once

namespace nvl::traits {

template <typename T>
concept HasDereferenceOperator = requires(T a) { *a; };

template <typename T>
concept HasMemberAccessOperator = requires(T a) { a.operator->(); };

} // namespace nvl::traits
