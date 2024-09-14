#pragma once

namespace nvl::traits {

template <typename T>
concept HasBox = requires(T a) { a.box(); };

} // namespace nvl::traits
