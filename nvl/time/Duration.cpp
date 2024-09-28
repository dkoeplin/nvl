#include "nvl/time/Duration.h"

#include <sstream>

namespace nvl {

pure std::string Duration::to_string() const {
    TimeScale scale = TimeScale::kNanoseconds;
    U64 time = nanos_;
    U64 divisor = 1;
    while (scale < TimeScale::kNScales - 1 && time > TimeScale::divisors[scale]) {
        const U64 factor = TimeScale::divisors[scale];
        scale = TimeScale(static_cast<TimeScale::Value>(scale.value + 1));
        divisor *= factor;
        time /= factor;
    }
    std::stringstream ss;
    ss << (static_cast<F64>(nanos_) / static_cast<F64>(divisor)) << " " << scale;
    return ss.str();
}

pure std::string Duration::raw() const {
    std::stringstream ss;
    ss << nanos_ << " ns";
    return ss.str();
}

} // namespace nvl
