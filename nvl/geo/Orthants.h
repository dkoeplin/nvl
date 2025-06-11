#pragma once

#include "nvl/data/Map.h"
#include "nvl/data/Maybe.h"
#include "nvl/geo/Tuple.h"
#include "nvl/geo/Volume.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/Pure.h"

namespace nvl {

/**
 * @struct Orthants
 * @brief "Orthants" is the ND version of 2D quadrants!
 * Covers 2^N orthants, originating at [origins] over [size] in each dimension.
 * * @tparam N - Number of dimensions
 */
template <U64 N>
struct Orthants {
    explicit Orthants(const Pos<N> &origin, const I64 size) : origin(origin), grid_size(size) {}

    static U64 nd_to_flat(const Pos<N> &delta) {
        static const Map<Pos<N>, U64> map = make_mapping();
        return map.at(delta);
    }

    template <typename VisitFunc> // (Pos<N>, U64) => void
    static void walk(VisitFunc func) {
        static constexpr Box<N> grid{Pos<N>::fill(0), Pos<N>::fill(2)};
        U64 flat_index = 0;
        for (const Pos<N> &index : grid.indices()) {
            Pos<N> delta = index * 2 - 1; // 1 => 1, 0 => -1
            func(delta, flat_index);
            ++flat_index;
        }
    }

    pure Box<N> bbox() const { return {origin - grid_size, origin + grid_size}; }
    pure Box<N> bound(const Pos<N> &delta) const { return Box<N>(origin, origin + delta * grid_size); }

    pure Pos<N> delta(const Pos<N> &pos) const {
        Pos<N> delta;
        simd for (U64 i = 0; i < N; ++i) { delta[i] = pos[i] >= origin[i] ? 1 : -1; }
        return delta;
    }

    /// Returns a flattened index which addresses a specific 2^N orthant.
    /// Returns None if [pos] is outside the bounds of this Orthants.
    pure Maybe<U64> index(const Pos<N> &pos) const {
        return_if(!bbox().contains(pos), None);
        return nd_to_flat(delta(pos));
    }

    Pos<N> origin;
    I64 grid_size;

private:
    static Map<Pos<N>, U64> make_mapping() {
        Map<Pos<N>, U64> map;
        Orthants<N>::walk([&](const Pos<N> &delta, const U64 idx) { map[delta] = idx; });
        return map;
    }
};

} // namespace nvl
