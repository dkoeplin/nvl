#pragma once

#include <random>

#include "nvl/macros/Aliases.h"
#include "nvl/macros/Expand.h"
#include "nvl/macros/Unreachable.h"

namespace nvl {

class Random {
public:
    Random() : seed(os_seed()), engine(seed) {}

    explicit Random(const U64 seed) : seed(seed), engine(seed) {}

    template <typename I>
        requires std::is_integral_v<I> || std::is_floating_point_v<I>
    expand I uniform(const I min, const I max) {
        return std::uniform_int_distribution<I>(min, max)(engine);
    }

    template <typename I>
        requires std::is_integral_v<I>
    expand I normal(const I mean, const I stddev) {
        return std::normal_distribution<I>(mean, stddev)(engine);
    }

    template <U64 N, typename I>
        requires std::is_integral_v<I>
    expand void uniform(std::array<I, N> &data, const I min, const I max) {
        std::uniform_int_distribution<I> distribution(min, max);
        for (U64 i = 0; i < N; ++i) {
            data[i] = distribution(engine);
        }
    }

    template <U64 N, typename I>
        requires std::is_integral_v<I> || std::is_floating_point_v<I>
    expand void normal(std::array<I, N> &data, const I mean, const I stddev) {
        std::normal_distribution<I> distribution(mean, stddev);
        for (U64 i = 0; i < N; ++i) {
            data[i] = distribution(engine);
        }
    }

    template <typename V, typename I>
    expand V uniform(const I min, const I max) {
        std::array<I, sizeof(V)> data;
        uniform<sizeof(V), I>(data, min, max);
        return *(V *)(&data);
    }

    template <typename V, typename I>
    expand V normal(const I min, const I max) {
        std::array<I, sizeof(V)> data;
        normal<sizeof(V), I>(data, min, max);
        return *(V *)(&data);
    }

private:
    std::random_device os_seed;
    U64 seed;
    std::mt19937 engine;
};

} // namespace nvl
