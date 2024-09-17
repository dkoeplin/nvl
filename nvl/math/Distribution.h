#pragma once

#include <memory>

#include "nvl/math/Random.h"

namespace nvl {

struct Distribution {
    enum Kind { kUniform, kNormal, kCustom };
    enum Type { kU64, kI64 };
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
    pure Kind kind() const;
    pure Type type() const;

    template <typename T>
    T next(Random &random) const;
};

struct Distribution::Impl {
    explicit Impl(const Kind kind, const Type type) : kind(kind), type(type) {}
    virtual ~Impl() = default;
    Kind kind;
    Type type;
};

inline Distribution::Kind Distribution::kind() const { return impl_->kind; }
inline Distribution::Type Distribution::type() const { return impl_->type; }

template <typename T>
pure Distribution::Type reflect() {
    UNREACHABLE();
}
template <>
pure inline Distribution::Type reflect<U64>() {
    return Distribution::Type::kU64;
}
template <>
pure inline Distribution::Type reflect<I64>() {
    return Distribution::Type::kI64;
}
/*template <>
pure inline Distribution::Type reflect<double>() {
    return Distribution::Type::kF64;
}*/

template <typename T>
struct DistributionUniform : Distribution::Impl {
    explicit DistributionUniform(T min, T max) : Impl(Distribution::Kind::kUniform, reflect<T>()), min(min), max(max) {}
    T min;
    T max;
};

template <typename T>
struct DistributionNormal : Distribution::Impl {
    explicit DistributionNormal(T mean, T stddev)
        : Impl(Distribution::Kind::kNormal, reflect<T>()), mean(mean), stddev(stddev) {}
    T mean;
    T stddev;
};

template <typename T>
struct DistributionCustom : Distribution::Impl {
    explicit DistributionCustom(const std::function<T(Random &)> &func)
        : Impl(Distribution::Kind::kCustom, Distribution::Type::kU64), func(func) {}
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
    switch (kind()) {
    case kUniform: {
        switch (type()) {
        case kU64: {
            const auto &dist = *static_cast<DistributionUniform<U64> *>(impl_.get());
            return random.uniform<T, U64>(dist.min, dist.max);
        }
        case kI64: {
            const auto &dist = *static_cast<DistributionUniform<I64> *>(impl_.get());
            return random.uniform<T, I64>(dist.min, dist.max);
        }
            /*case kF64: {
                const auto &dist = *static_cast<DistributionUniform<double> *>(impl_.get());
                return random.uniform<T, double>(dist.min, dist.max);
            }*/
        }
    }
    case kNormal: {
        switch (type()) {
        case kU64: {
            const auto &dist = *static_cast<DistributionNormal<U64> *>(impl_.get());
            return random.normal<T, U64>(dist.mean, dist.stddev);
        }
        case kI64: {
            const auto &dist = *static_cast<DistributionNormal<I64> *>(impl_.get());
            return random.normal<T, I64>(dist.mean, dist.stddev);
        }
            /*case kF64: {
                const auto &dist = *static_cast<DistributionNormal<double> *>(impl_.get());
                return random.normal<T, double>(dist.mean, dist.stddev);
            }*/
        }
    }
    case kCustom: {
        const auto &dist = *static_cast<DistributionCustom<T> *>(impl_.get());
        return dist.func(random);
    }
    default:
        UNREACHABLE();
    }
}

} // namespace nvl
