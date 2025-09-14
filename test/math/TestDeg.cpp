#include <gtest/gtest.h>

#include "nvl/math/Trig.h"
#include "nvl/time/Clock.h"
#include "nvl/time/Duration.h"

namespace {

using nvl::Clock;
using nvl::Deg;
using nvl::Duration;
using nvl::List;
using nvl::Pos;
using nvl::Vec;

template <typename LutFunc, typename StdFunc>
void test(const std::string &name, LutFunc lutfunc, StdFunc stdfunc) {
    List<F64> rads(Deg::kDegreeMax, 0);
    List<F64> lut(Deg::kDegreeMax, 0);
    List<F64> stl(Deg::kDegreeMax, 0);
    const auto pop_start = Clock::now();
    std::cout << lutfunc(Deg::make_raw(32)) << std::endl; // Populate LUT
    const auto pop_time = Duration(Clock::now() - pop_start);

    for (I64 d = 0; d < Deg::kDegreeMax; ++d) {
        rads[d] = Deg::make_raw(d).radians();
    }

    const auto lut_start = Clock::now();
    for (I64 d = 0; d < Deg::kDegreeMax; ++d) {
        const Deg deg = Deg::make_raw(d);
        lut[d] = lutfunc(deg);
    }
    const auto lut_time = Duration(Clock::now() - lut_start);

    const auto std_start = Clock::now();
    for (I64 d = 0; d < Deg::kDegreeMax; ++d) {
        stl[d] = stdfunc(rads[d]);
    }
    const auto std_time = Duration(Clock::now() - std_start);

    for (I64 d = 0; d < Deg::kDegreeMax; ++d) {
        EXPECT_EQ(stl[d], lut[d]);
    }
    std::cout << name << ": " << std::endl;
    std::cout << "  pop: " << pop_time << " (" << pop_time / Deg::kDegreeMax << "/call)" << std::endl;
    std::cout << "  lut: " << lut_time << " (" << lut_time / Deg::kDegreeMax << "/call)" << std::endl;
    std::cout << "  std: " << std_time << " (" << std_time / Deg::kDegreeMax << "/call)" << std::endl;
}

TEST(TestDeg, sin) {
    test("sin", [](Deg deg) { return nvl::sin(deg); }, [](F64 rad) { return std::sin(rad); });
}

TEST(TestDeg, cos) {
    test("cos", [](Deg deg) { return nvl::cos(deg); }, [](F64 rad) { return std::cos(rad); });
}

TEST(TestDeg, tan) {
    test("tan", [](Deg deg) { return nvl::tan(deg); }, [](F64 rad) { return std::tan(rad); });
}

} // namespace
