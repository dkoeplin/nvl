#pragma once

#include <gtest/gtest.h>

#include <tuple>

#include "nvl/data/List.h"
#include "nvl/data/Map.h"
#include "nvl/math/Distribution.h"
#include "nvl/math/Random.h"
#include "nvl/time/Duration.h"

namespace nvl::testing {

template <U64 i, typename Tuple>
void generate(Tuple &, Random &, const Map<U64, Distribution> &) {
    // Don't do anything!
}

template <U64 i, typename Tuple, typename Arg, typename... Args>
void generate(Tuple &tuple, Random &random, const Map<U64, Distribution> &distributions) {
    const auto &distribution = distributions.at(i - 1);
    std::get<i>(tuple) = distribution.next<Arg>(random);
    generate<i + 1, Tuple, Args...>(tuple, random, distributions);
}

template <typename Result, typename... Args>
class Fuzzer {
public:
    using Clock = std::chrono::steady_clock;
    using Tuple = std::tuple<Result, Args...>;

    Distribution &operator[](const U64 i) { return in[i]; }

    template <typename Func>
    void fuzz(Func func) {
        for (U64 i = 0; i < N; ++i) {
            Tuple tuple;
            generate<1, Tuple, Args...>(tuple, random_, in);
            io.push_back(std::move(tuple));
        }

        const auto start = Clock::now();
        for (U64 i = 0; i < N; ++i) {
            std::apply(func, io[i]);
        }
        const auto end = Clock::now();

        const Duration time(end - start);
        std::cout << "Over " << N << " calls:" << std::endl;
        std::cout << "  Total Time:      " << time << std::endl;
        std::cout << "  Avg. Call Time:  " << (time / N) << std::endl;
    }

    template <typename Func>
    void verify(Func func) {
        for (U64 i = 0; i < N; ++i) {
            std::apply(func, io[i]);
        }
    }

    U64 N = 1E6;
    Map<U64, Distribution> in;

private:
    Random random_;
    List<Tuple> io;
};

template <typename Result, typename... Args>
class FuzzingTestFixture : public Fuzzer<Result, Args...>, public ::testing::Test {};

} // namespace nvl::testing
