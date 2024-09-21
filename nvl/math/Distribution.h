#pragma once

#include <memory>

#include "nvl/macros/Unreachable.h"
#include "nvl/math/Random.h"
#include "nvl/reflect/Casting.h"
#include "nvl/reflect/ClassTag.h"
#include "nvl/reflect/PrimitiveTypes.h"

namespace nvl {

struct Distribution {
    struct Impl;

    template <typename T>
    static Distribution Uniform(T min, T max);

    template <typename T>
    static Distribution Normal(T mean, T stddev);

    template <typename T>
    static Distribution Custom(const std::function<T(Random &)> &func);

    Distribution() = default;
    explicit Distribution(std::shared_ptr<Impl> impl) : impl_(std::move(impl)) {}

    std::shared_ptr<Impl> impl_ = nullptr;

    template <typename T>
    const T *dyn_cast() const {
        return nvl::dyn_cast<T>(impl_.get());
    }

    template <typename T>
    T next(Random &random) const;
};

struct Distribution::Impl {
    class_tag(Distribution::Impl);
    virtual ~Impl() = default;
};

template <typename T>
struct DistributionUniform : Distribution::Impl {
    class_tag(DistributionUniform<T>);
    explicit DistributionUniform(T min, T max) : min(min), max(max) {}
    T min;
    T max;
};

template <typename T>
struct DistributionNormal : Distribution::Impl {
    class_tag(DistributionNormal<T>);
    explicit DistributionNormal(T mean, T stddev) : mean(mean), stddev(stddev) {}
    T mean;
    T stddev;
};

template <typename T>
struct DistributionCustom : Distribution::Impl {
    class_tag(DistributionCustom<T>);
    explicit DistributionCustom(const std::function<T(Random &)> &func) : func(func) {}
    std::function<T(Random &)> func;
};

template <typename T>
Distribution Distribution::Uniform(T min, T max) {
    return Distribution(std::make_shared<DistributionUniform<T>>(min, max));
}

template <typename T>
Distribution Distribution::Normal(T mean, T stddev) {
    return Distribution(std::make_shared<DistributionNormal<T>>(mean, stddev));
}

template <typename T>
Distribution Distribution::Custom(const std::function<T(Random &)> &func) {
    return Distribution(std::make_shared<DistributionCustom<T>>(func));
}

template <typename T>
T Distribution::next(Random &random) const {
    if (const auto *uniform_bool = dyn_cast<DistributionUniform<bool>>()) {
        return random.uniform<T, bool>(uniform_bool->min, uniform_bool->max);
    } else if (const auto *uniform_u64 = dyn_cast<DistributionUniform<U64>>()) {
        return random.uniform<T, U64>(uniform_u64->min, uniform_u64->max);
    } else if (const auto *uniform_i64 = dyn_cast<DistributionUniform<I64>>()) {
        return random.uniform<T, I64>(uniform_i64->min, uniform_i64->max);
    } else if (const auto *uniform_f64 = dyn_cast<DistributionUniform<F64>>()) {
        return random.uniform<T, F64>(uniform_f64->min, uniform_f64->max);
    } else if (const auto *normal_u64 = dyn_cast<DistributionNormal<U64>>()) {
        return random.normal<T, U64>(normal_u64->mean, normal_u64->stddev);
    } else if (const auto *normal_i64 = dyn_cast<DistributionNormal<I64>>()) {
        return random.normal<T, I64>(normal_i64->mean, normal_i64->stddev);
    } else if (const auto *normal_f64 = dyn_cast<DistributionNormal<F64>>()) {
        return random.normal<T, F64>(normal_f64->mean, normal_f64->stddev);
    } else if (const auto *custom = dyn_cast<DistributionCustom<T>>()) {
        return custom->func(random);
    }
    UNREACHABLE();
}

} // namespace nvl
