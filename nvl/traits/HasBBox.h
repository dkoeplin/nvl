#pragma once

namespace nvl::traits {

template <typename T>
concept HasBBox = requires(T a) { a.bbox(); };

} // namespace nvl::traits
