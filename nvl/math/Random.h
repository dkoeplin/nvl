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
    expand I uniform(const I min, const I max) {
        return std::uniform_int_distribution<I>(min, max)(engine);
    }

    template <typename I>
    expand I normal(const I mean, const I stddev) {
        return std::normal_distribution<I>(mean, stddev)(engine);
    }

private:
    std::random_device os_seed;
    U64 seed;
    std::mt19937 engine;
};

} // namespace nvl
