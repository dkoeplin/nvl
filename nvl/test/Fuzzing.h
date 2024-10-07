#pragma once

#include <gtest/gtest.h>

#include <tuple>

#include "nvl/data/List.h"
#include "nvl/data/Map.h"
#include "nvl/math/Distribution.h"
#include "nvl/math/Random.h"
#include "nvl/time/Duration.h"

namespace nvl::test {

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

    Fuzzer() = default;
    explicit Fuzzer(const U64 seed) : random_(seed) {}

    Distribution &operator[](const U64 i) { return in[i]; }

    template <typename Func>
    void fuzz(Func func) {
        for (U64 i = 0; i < num_tests; ++i) {
            Tuple tuple;
            generate<1, Tuple, Args...>(tuple, random_, in);
            io.push_back(std::move(tuple));
        }

        const auto start = Clock::now();
        for (U64 i = 0; i < num_tests; ++i) {
            std::apply(func, io[i]);
        }
        const auto end = Clock::now();

        const Duration time(end - start);
        std::cout << "Over " << num_tests << " calls:" << std::endl;
        std::cout << "  Total call time:  " << time << std::endl;
        std::cout << "  Avg. time / call: " << (time / num_tests) << std::endl;
    }

    template <typename Func>
    void verify(Func func) {
        const auto start = Clock::now();
        for (U64 i = 0; i < num_tests; ++i) {
            std::apply(func, io[i]);
        }
        const auto end = Clock::now();
        const Duration time(end - start);
        std::cout << "Verify time: " << time << std::endl;
        std::cout << "Verify time / Call: " << (time / num_tests) << std::endl;
    }

    U64 num_tests = 1E6;
    Map<U64, Distribution> in;

private:
    Random random_;
    List<Tuple> io;
};

template <typename Result, typename... Args>
class FuzzingTestFixture : public Fuzzer<Result, Args...>, public ::testing::Test {};

} // namespace nvl::test
