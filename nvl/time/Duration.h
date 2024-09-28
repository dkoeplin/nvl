#pragma once

#include <string>

#include "nvl/macros/Aliases.h"
#include "nvl/time/TimeScale.h"

namespace nvl {

/**
 * @class Duration
 * @brief Represents a duration of time in nanoseconds.
 */
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

    pure std::string to_string() const;

    pure std::string raw() const;

private:
    U64 nanos_;
};

inline std::ostream &operator<<(std::ostream &os, const Duration &duration) { return os << duration.to_string(); }

} // namespace nvl
