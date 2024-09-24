#pragma once

#include <random>

#include "nvl/macros/Aliases.h"
#include "nvl/macros/Expand.h"
#include "nvl/macros/Pure.h"

namespace nvl {

template <typename V>
struct RandomGen;

class Random {
public:
    Random() : seed_(os_seed_()), engine_(seed_) {}

    explicit Random(const U64 seed) : seed_(seed), engine_(seed) {}

    template <typename I>
        requires std::is_integral_v<I>
    expand I uniform(const I min, const I max) {
        return std::uniform_int_distribution<I>(min, max)(engine_);
    }

    expand F64 uniform(const F64 min, const F64 max) { return std::uniform_real_distribution(min, max)(engine_); }

    expand double normal(const double mean, const double stddev) {
        return std::normal_distribution(mean, stddev)(engine_);
    }

    template <typename V, typename I, typename Gen = RandomGen<V>>
    expand V uniform(I min, I max);

    template <typename V, typename I, typename Gen = RandomGen<V>>
    expand V normal(I mean, I stddev);

    std::mt19937 &engine() { return engine_; }

private:
    std::random_device os_seed_;
    const U64 seed_;
    std::mt19937 engine_;
};

template <typename V>
struct RandomGen {
    template <typename I>
        requires std::is_integral_v<I> || std::is_floating_point_v<I> || std::is_same_v<I, bool>
    pure V uniform(Random &random, const I min, const I max) const {
        std::array<I, sizeof(V)> data;
        if constexpr (std::is_same_v<I, bool>) {
            std::uniform_int_distribution distribution(static_cast<U64>(min), static_cast<U64>(max));
            for (U64 i = 0; i < sizeof(V); ++i) {
                data[i] = static_cast<bool>(distribution(random.engine()));
            }
        } else if constexpr (std::is_floating_point_v<I>) {
            std::uniform_real_distribution<I> distribution(min, max);
            for (U64 i = 0; i < sizeof(V); ++i) {
                data[i] = distribution(random.engine());
            }
        } else {
            std::uniform_int_distribution<I> distribution(min, max);
            for (U64 i = 0; i < sizeof(V); ++i) {
                data[i] = distribution(random.engine());
            }
        }
        return *(V *)(&data);
    }
    pure V normal(Random &random, const double mean, const double stddev) const {
        std::array<double, sizeof(V)> data;
        std::normal_distribution distribution(mean, stddev);
        for (U64 i = 0; i < sizeof(V); ++i) {
            data[i] = distribution(random.engine());
        }
        return *(V *)(&data);
    }
};

template <typename V, typename I, typename Gen>
expand V Random::uniform(const I min, const I max) {
    return Gen().uniform(*this, min, max);
}

template <typename V, typename I, typename Gen>
expand V Random::normal(const I mean, const I stddev) {
    return Gen().normal(*this, mean, stddev);
}

} // namespace nvl
