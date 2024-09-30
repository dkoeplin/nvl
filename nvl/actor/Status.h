#pragma once

#include <ostream>

namespace nvl {

enum class Status { kNone, kIdle, kMove, kDied };

std::ostream &operator<<(std::ostream &os, Status status);

} // namespace nvl
