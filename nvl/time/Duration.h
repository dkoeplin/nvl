#pragma once

#include <string>

#include "nvl/data/Maybe.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/Expand.h"
#include "nvl/time/Clock.h"
#include "nvl/time/TimeScale.h"

namespace nvl {

/**
 * @class Duration
 * @brief Represents a duration of time in nanoseconds.
 */
class Duration {
public:
    Duration() = default;

    explicit Duration(const TimeDiff &time) {
        nanos_ = std::chrono::duration_cast<std::chrono::nanoseconds>(time).count();
    }

    explicit Duration(const I64 nanos) : nanos_(nanos) {}

    Duration &operator=(const TimeDiff &time) {
        nanos_ = std::chrono::duration_cast<std::chrono::nanoseconds>(time).count();
        str_ = None;
        return *this;
    }

    implicit operator std::chrono::nanoseconds() const { return std::chrono::nanoseconds(nanos_); }

    pure Duration operator+(const Duration &rhs) const { return Duration(nanos_ + rhs.nanos_); }
    pure Duration operator-(const Duration &rhs) const { return Duration(nanos_ - rhs.nanos_); }

    pure Duration operator*(const I64 rhs) const { return Duration(nanos_ * rhs); }
    pure Duration operator/(const I64 rhs) const { return Duration(nanos_ / rhs); }
    pure Duration operator+(const I64 rhs) const { return Duration(nanos_ + rhs); }
    pure Duration operator-(const I64 rhs) const { return Duration(nanos_ - rhs); }

    pure bool operator==(const Duration &rhs) const { return nanos_ == rhs.nanos_; }
    pure bool operator!=(const Duration &rhs) const { return nanos_ != rhs.nanos_; }
    pure bool operator>=(const Duration &rhs) const { return nanos_ >= rhs.nanos_; }
    pure bool operator>(const Duration &rhs) const { return nanos_ > rhs.nanos_; }
    pure bool operator<=(const Duration &rhs) const { return nanos_ <= rhs.nanos_; }
    pure bool operator<(const Duration &rhs) const { return nanos_ < rhs.nanos_; }

    pure bool operator==(const I64 rhs) const { return nanos_ == rhs; }
    pure bool operator!=(const I64 rhs) const { return nanos_ != rhs; }
    pure bool operator>=(const I64 rhs) const { return nanos_ >= rhs; }
    pure bool operator>(const I64 rhs) const { return nanos_ > rhs; }
    pure bool operator<=(const I64 rhs) const { return nanos_ <= rhs; }
    pure bool operator<(const I64 rhs) const { return nanos_ < rhs; }

    pure std::string to_string() const;

    pure I64 nanos() const { return nanos_; }

private:
    I64 nanos_ = 0;
    mutable Maybe<std::string> str_ = None;
};

inline std::ostream &operator<<(std::ostream &os, const Duration &duration) { return os << duration.to_string(); }

expand Duration max(const Duration &a, const Duration &b) { return a >= b ? a : b; }
expand Duration min(const Duration &a, const Duration &b) { return a <= b ? a : b; }

} // namespace nvl
