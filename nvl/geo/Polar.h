#pragma once

#include "nvl/geo/Tuple.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/Expand.h"
#include "nvl/macros/Pure.h"
#include "nvl/math/Rotation.h"
#include "nvl/math/Trig.h"

namespace nvl {

/**
 * @class Polar
 * @brief A point in N-dimensional space expressed in polar coordinates.
 * The point may be relative to the origin or to an externally defined center point.
 */
template <U64 N>
class Polar {
public:
    explicit Polar(const Rotation<N> &theta, const F64 dist) : theta_(theta), dist_(dist) {}
    explicit Polar(const Vec<N> &center, const Vec<N> &point) {
        const Vec<N> rel = point - center;
        dist_ = rel.magnitude();
        theta_ = get_rotation(rel);
    }

    expand void operator+=(const Rotation<N> &delta) { theta_ += delta; }
    expand void operator-=(const Rotation<N> &delta) { theta_ -= delta; }
    expand void operator+=(const F64 delta) { dist_ += delta; }
    expand void operator-=(const F64 delta) { dist_ -= delta; }

    pure expand Polar operator+(const Rotation<N> &delta) const { return Polar(theta_ + delta, dist_); }
    pure expand Polar operator-(const Rotation<N> &delta) const { return Polar(theta_ - delta, dist_); }
    pure expand Polar operator+(const F64 delta) const { return Polar(theta_, dist_ + delta); }
    pure expand Polar operator-(const F64 delta) const { return Polar(theta_, dist_ - delta); }

    pure expand const Rotation<N> &theta() const { return theta_; }
    pure expand F64 dist() const { return dist_; }

    /// Returns the equivalent this point in cartesian coordinates when relative to the given center point.
    pure expand Vec<N> to_cartesian(const Vec<N> &center) const {
        Vec<N> pt = center;
        for (const Plane &plane : Plane::all<N>()) {
            pt[plane.axis0] += dist_ * cos(theta_[plane]);
            pt[plane.axis1] += dist_ * sin(theta_[plane]);
        }
        return pt;
    }

    pure std::string to_string() const {
        std::stringstream ss;
        ss << theta_ << " @ " << dist_;
        return ss.str();
    }

private:
    Rotation<N> theta_; // N-dimensional angle from center point
    F64 dist_;          // Distance from the center point
};

template <U64 N>
std::ostream &operator<<(std::ostream &os, const Polar<N> &polar) {
    return os << polar.to_string();
}

} // namespace nvl
