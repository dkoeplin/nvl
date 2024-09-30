#pragma once

#include <ostream>

namespace nvl::trait {

template <typename T>
concept HasPrint = requires(std::ostream &os, T v) { os << v; };

} // namespace nvl::trait