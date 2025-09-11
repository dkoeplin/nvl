#pragma once

#include "nvl/data/Counter.h"
#include "nvl/geo/Volume.h"
#include "nvl/macros/Aliases.h"
#include "nvl/math/FastTrig.h"

namespace nvl {

/**
 * @class RBox
 * @brief A box with rotation in N-dimensional space.
 */
template <U64 N>
class RBox {
public:
    explicit RBox(Box<N> box) : box_(box) {}

    expand void rotate(const fasttrig::Rot<N> rotation) { return rot_ += rotation; }

    pure List<Vec<N>> points() const {
        List<Vec<N>> points;
        const Vec<N> shape = real(box_.shape());
        const Vec<N> min = real(box_.min) - shape / 2;
        const Vec<N> max = box_.max_f64() - shape / 2;
        for (const List<U64> &point : Counter<U64>::get(N, 2)) {
            Vec<N> pt;
            simd for (U64 i = 0; i < N; ++i) { pt[i] = point[i] == 0 ? min[i] : max[i]; }
            points.push_back(fasttrig::rotate(pt, rot_));
        }
        return points;
    }

private:
    Box<N> box_;
    fasttrig::Rot<N> rot_;
};

} // namespace nvl