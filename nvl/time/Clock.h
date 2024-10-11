#pragma once

#include <chrono>

namespace nvl {

using Clock = std::chrono::steady_clock;
using Time = std::chrono::time_point<std::chrono::steady_clock>;
using TimeDiff = std::chrono::steady_clock::duration;

} // namespace nvl
