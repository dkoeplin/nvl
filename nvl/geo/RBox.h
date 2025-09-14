#pragma once

#include "nvl/data/Counter.h"
#include "nvl/geo/Volume.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/ReturnIf.h"
#include "nvl/math/Rotation.h"
#include "nvl/math/Trig.h"

namespace nvl {

/**
 * @class RBox
 * @brief A box with rotation in N-dimensional space.
 */
template <U64 N>
class RBox {
public:
    constexpr explicit RBox(const Box<N> &box, const Rotation<N> &rot) : box_(box), rot_(rot) {}

    pure expand RBox operator+(const Pos<N> &delta) const { return RBox(box_ + delta, rot_); }
    pure expand RBox operator-(const Pos<N> &delta) const { return RBox(box_ - delta, rot_); }

    expand RBox &rotate(const Rotation<N> &rotation) {
        changed_ = true;
        rot_ += rotation;
        return *this;
    }

    RBox &operator+=(const Pos<N> &delta) {
        update_points();
        box_ += delta;
        points_ += delta;
        return *this;
    }
    RBox &operator-=(const Pos<N> &delta) {
        update_points();
        box_ -= delta;
        points_ -= delta;
        return *this;
    }

    pure expand const Tuple<1 << N, Vec<N>> &points() const {
        update_points();
        return points_;
    }

    pure expand std::string to_string() const {
        std::stringstream ss;
        ss << box_ << " @ " << rot_;
        return ss.str();
    }

private:
    void update_points() const;

    Box<N> box_;
    Rotation<N> rot_;
    mutable Tuple<1 << N, Vec<N>> points_;
    mutable bool changed_ = true;
};

template <U64 N>
void RBox<N>::update_points() const {
    return_if(!changed_);

    changed_ = false;
    const Vec<N> radius = real(box_.shape()) / 2;
    const Vec<N> min = real(box_.min) - radius;
    const Vec<N> max = box_.max_f64() - radius;
    U64 p = 0;
    for (const List<U64> &point : Counter<U64>::get(N, 2)) {
        Vec<N> pt;
        simd for (U64 i = 0; i < N; ++i) { pt[i] = point[i] == 0 ? min[i] : max[i]; }
        points_[p] = nvl::rotate(pt, rot_) + radius;
        ++p;
    }
}

template <U64 N>
std::ostream &operator<<(std::ostream &os, const RBox<N> &box) {
    return os << box.to_string();
}

} // namespace nvl