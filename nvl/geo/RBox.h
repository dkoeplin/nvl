#pragma once

#include "nvl/data/Counter.h"
#include "nvl/geo/Polar.h"
#include "nvl/geo/Volume.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/ReturnIf.h"
#include "nvl/math/Bitwise.h"
#include "nvl/math/Rotation.h"

namespace nvl {

/**
 * @class RBox
 * @brief A box with rotation in N-dimensional space.
 */
template <U64 N>
class RBox {
public:
    static constexpr U64 P = 1 << N; // Number of points in this box

    explicit RBox(const Pos<N> &shape, const Vec<N> &center) : shape_(shape), center_(center), rot_(Rotation<N>::zero) {
        const Vec<N> radius = real(shape_) / 2;
        const Vec<N> min = center + radius;
        const Vec<N> max = center - radius;

        for (U64 p = 0; p < P; ++p) {
            Vec<N> pt;
            simd for (U64 i = 0; i < N; ++i) { pt[i] = (p & (1 << i)) ? min[i] : max[i]; }
            polar_[p] = Polar(center_, pt);
        }
    }

    explicit RBox(const Pos<N> &shape, const Vec<N> &center, const Rotation<N> &rot) : RBox(shape, center) {
        rotate(rot);
    }

    explicit RBox(const Box<N> &box, const Rotation<N> &rot)
        : RBox(box.shape(), real(box.min) + real(box.shape() / 2), rot) {}

    pure expand RBox operator+(const Pos<N> &delta) const { return RBox(shape_, center_ + delta, rot_); }
    pure expand RBox operator-(const Pos<N> &delta) const { return RBox(shape_, center_ - delta, rot_); }

    pure expand bool empty() const {
        return shape_.exists([](const I64 x) { return x == 0; });
    }

    /// Rotates this box with respect to its center point.
    expand RBox &rotate(const Rotation<N> &rotation) {
        has_rotated_ = true;
        rot_ += rotation;
        polar_ += rotation;
        return *this;
    }

    /// Rotates this box with respect to the given point.
    RBox &rotate(const Rotation<N> &rotation, const Vec<N> &point) {
        has_rotated_ = true;
        center_ = nvl::rotate(center_ - point, rotation) + point;
        rot_ += rotation;
        polar_ += rotation;
        return *this;
    }

    /// Returns a copy of this RBox, rotated around its center point by [rotation].
    RBox rotated(const Rotation<N> &rotation) {
        RBox result = *this; // copy
        result.rotate(rotation);
        return result;
    }

    /// Rotates this box with respect to the given point.
    RBox rotated(const Rotation<N> &rotation, const Vec<N> &point) {
        RBox result = *this; // copy
        result.rotate(rotation, point);
        return result;
    }

    RBox &operator+=(const Pos<N> &delta) {
        update_points();
        center_ += delta;
        points_ += delta;
        return *this;
    }
    RBox &operator-=(const Pos<N> &delta) {
        update_points();
        center_ -= delta;
        points_ -= delta;
        return *this;
    }

    /// Iterates over all lines in this box, calling [func] on each line.
    template <typename Func>
    void walk_lines(Func func) const {
        update_points();
        // An N-dimensional box has 2^N points.
        // Each point is connected to N other points.
        // To avoid repeats, only start from even parity indices.
        // Identify connected points by those that are a single bit flip in index away.
        // This works because points are populated such that, for dimension i, index bit i denotes (0=min, 1=max)
        // e.g. in 3D, 000 = minimum x,y,z
        // 000 is connected to 001, 010, and 100
        for (const U64 p : even_parity<N>()) {
            for (U64 i = 0; i < N; ++i) {
                const U64 q = p ^ (0x1 << i);
                const LineView line(points_[p], points_[q]);
                return_if(func(line) == WalkResult::kExit);
            }
        }
    }

    pure expand std::string to_string() const {
        std::stringstream ss;
        ss << shape_ << " @ " << rot_;
        return ss.str();
    }

    /// Returns the center of this box.
    pure expand const Vec<N> &center() const { return center_; }

    /// Returns the edge points of this box, in polar coordinates relative to the center.
    pure expand const Tuple<P, Polar<N>> &polar() const { return polar_; }

    /// Returns the rotation of this box.
    pure expand const Rotation<N> &rotation() const { return rot_; }

    /// Returns the edge points of this box in absolute cartesian coordinates.
    pure expand const Tuple<P, Vec<N>> &points() const {
        update_points();
        return points_;
    }

    /// Returns the non-rotated volume around the center point.
    pure expand Volume<N, F64> raw_volume() const {
        const Tuple<N, F64> radius = real(shape_) / 2;
        const Tuple<N, F64> min = center_ - radius;
        const Tuple<N, F64> max = center_ + radius;
        return Volume<N, F64>(min, max);
    }

    pure expand Box<N> bbox() const {
        update_points();
        Pos<N> a = floor(points_[0]);
        Pos<N> b = ceil(points_[0]);
        for (U64 i = 1; i < P; ++i) {
            a = min(a, floor(points_[i]));
            b = max(b, ceil(points_[i]));
        }
        return Box<N>(a, b);
    }

private:
    void update_points() const;
    Pos<N> shape_; // Shape (without rotation)
    Vec<N> center_;
    Rotation<N> rot_;
    Tuple<1 << N, Polar<N>> polar_;
    mutable bool has_rotated_ = true;
    mutable Tuple<1 << N, Vec<N>> points_;
};

template <size_t N>
void RBox<N>::update_points() const {
    return_if(!has_rotated_);
    has_rotated_ = false;
    for (U64 i = 0; i < polar_.rank(); ++i) {
        points_[i] = polar_[i].to_cartesian(center_);
    }
}

template <U64 N>
std::ostream &operator<<(std::ostream &os, const RBox<N> &box) {
    return os << box.to_string();
}

} // namespace nvl