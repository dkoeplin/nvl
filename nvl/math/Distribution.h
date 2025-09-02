#pragma once

#include <memory>

#include "nvl/macros/Abstract.h"
#include "nvl/macros/Unreachable.h"
#include "nvl/math/Random.h"
#include "nvl/reflect/CastableShared.h"
#include "nvl/reflect/ClassTag.h"

namespace nvl {

class Distribution;

abstract struct AbstractDistribution : CastableShared<Distribution, AbstractDistribution>::BaseClass {
    class_tag(AbstractDistribution);
};

/**
 * @class Distribution
 * @brief A random distribution for generating random numbers.
 *
 * Example Usage:
 *   auto distribution = Distribution::Uniform(0, 100);
 */
class Distribution final : public CastableShared<Distribution, AbstractDistribution> {
public:
    template <typename T>
    static Distribution Uniform(T min, T max);

    template <typename T>
    static Distribution Normal(T mean, T stddev);

    template <typename T>
    static Distribution Custom(const std::function<T(Random &)> &func);

    // TODO(davidk): This is a little incongruous. You can create a Distribution with type A and then request
    //               a value of type B from it. Is there any way to make this type checked?
    template <typename T>
    T next(Random &random) const;

private:
    using CastableShared::CastableShared;
};

template <typename T>
struct UniformDistribution final : AbstractDistribution {
    class_tag(UniformDistribution<T>, AbstractDistribution);
    explicit UniformDistribution(T min, T max) : min(min), max(max) {}
    T min;
    T max;
};

template <typename T>
struct NormalDistribution final : AbstractDistribution {
    class_tag(NormalDistribution<T>, AbstractDistribution);
    explicit NormalDistribution(T mean, T stddev) : mean(mean), stddev(stddev) {}
    T mean;
    T stddev;
};

template <typename T>
struct CustomDistribution final : AbstractDistribution {
    class_tag(CustomDistribution<T>, AbstractDistribution);
    explicit CustomDistribution(const std::function<T(Random &)> &func) : func(func) {}
    std::function<T(Random &)> func;
};

template <typename T>
Distribution Distribution::Uniform(T min, T max) {
    return Distribution(std::make_shared<UniformDistribution<T>>(min, max));
}

template <typename T>
Distribution Distribution::Normal(T mean, T stddev) {
    return Distribution(std::make_shared<NormalDistribution<T>>(mean, stddev));
}

template <typename T>
Distribution Distribution::Custom(const std::function<T(Random &)> &func) {
    return Distribution(std::make_shared<CustomDistribution<T>>(func));
}

template <typename T>
T Distribution::next(Random &random) const {
    if (const auto *uniform_bool = dyn_cast<UniformDistribution<bool>>())
        return random.uniform<T, bool>(uniform_bool->min, uniform_bool->max);
    if (const auto *uniform_u64 = dyn_cast<UniformDistribution<U64>>())
        return random.uniform<T, U64>(uniform_u64->min, uniform_u64->max);
    if (const auto *uniform_i64 = dyn_cast<UniformDistribution<I64>>())
        return random.uniform<T, I64>(uniform_i64->min, uniform_i64->max);
    if (const auto *uniform_f64 = dyn_cast<UniformDistribution<F64>>())
        return random.uniform<T, F64>(uniform_f64->min, uniform_f64->max);
    if (const auto *normal_u64 = dyn_cast<NormalDistribution<U64>>())
        return random.normal<T, U64>(normal_u64->mean, normal_u64->stddev);
    if (const auto *normal_i64 = dyn_cast<NormalDistribution<I64>>())
        return random.normal<T, I64>(normal_i64->mean, normal_i64->stddev);
    if (const auto *normal_f64 = dyn_cast<NormalDistribution<F64>>())
        return random.normal<T, F64>(normal_f64->mean, normal_f64->stddev);
    if (const auto *custom = dyn_cast<CustomDistribution<T>>())
        return custom->func(random);
    UNREACHABLE;
}

} // namespace nvl
