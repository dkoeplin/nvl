#pragma once

#include <memory>

#include "nvl/math/Random.h"

namespace nvl {

struct Distribution {
    enum Kind { kUniform, kNormal };
    struct Impl;

    template <typename T>
    static Distribution Uniform(T min, T max);

    template <typename T>
    static Distribution Normal(T mean, T stddev);

    Distribution() = default;
    explicit Distribution(std::shared_ptr<Impl> impl) : impl_(std::move(impl)) {}

    std::shared_ptr<Impl> impl_ = nullptr;

    template <typename T>
    T next(Random &random) const;
};

struct Distribution::Impl {};

template <typename T>
struct DistributionImpl : Distribution::Impl {
    explicit DistributionImpl(Distribution::Kind kind, T a, T b) : kind(kind), a(a), b(b) {}
    Distribution::Kind kind;
    T a;
    T b;
};

template <typename T>
Distribution Distribution::Uniform(T min, T max) {
    return Distribution(std::make_shared<DistributionImpl<T>>(kUniform, min, max));
}

template <typename T>
Distribution Distribution::Normal(T mean, T stddev) {
    return Distribution(std::make_shared<DistributionImpl<T>>(kNormal, mean, stddev));
}

template <typename T>
T Distribution::next(Random &random) const {
    const auto *dist = static_cast<DistributionImpl<T> *>(impl_.get());
    switch (dist->kind) {
    case kUniform:
        return random.uniform(dist->a, dist->b);
    case kNormal:
        return random.normal(dist->a, dist->b);
    default:
        UNREACHABLE();
    }
}

} // namespace
