#pragma once

namespace nvl::traits {

template <typename T>
concept HasID = requires(T a) { a.id(); };

} // namespace nvl::traits
