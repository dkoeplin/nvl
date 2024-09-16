#pragma once

#include "nvl/macros/Aliases.h"
#include "nvl/time/TimeScale.h"

namespace nvl {

class Duration {
public:
    template <typename Duration>
    explicit Duration(const Duration &duration) {
        nanos_ = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
    }
    explicit Duration(const U64 nanos) : nanos_(nanos) {}

    pure Duration operator*(const U64 rhs) const { return Duration(nanos_ * rhs); }
    pure Duration operator/(const U64 rhs) const { return Duration(nanos_ / rhs); }
    pure Duration operator+(const U64 rhs) const { return Duration(nanos_ + rhs); }
    pure Duration operator-(const U64 rhs) const { return Duration(nanos_ - rhs); }

    pure std::string to_string() const {
        TimeScale scale = TimeScale::kNanoseconds;
        U64 time = nanos_;
        double divisor = 1;
        while (scale < TimeScale::kNScales - 1 && time > TimeScale::divisors[scale]) {
            scale = TimeScale(static_cast<TimeScale::Value>(scale.value + 1));
            divisor *= static_cast<double>(TimeScale::divisors[scale]);
            time /= TimeScale::divisors[scale];
        }
        std::stringstream ss;
        ss << (nanos_ / divisor) << " " << scale;
        return ss.str();
    }

    pure std::string raw() const {
        std::stringstream ss;
        ss << nanos_ << " ns";
        return ss.str();
    }

private:
    U64 nanos_;
};

inline std::ostream &operator<<(std::ostream &os, const Duration &duration) { return os << duration.to_string(); }

} // namespace nvl