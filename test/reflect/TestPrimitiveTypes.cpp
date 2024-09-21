#include <gtest/gtest.h>

#include "nvl/reflect/PrimitiveTypes.h"

namespace {

TEST(TestPrimitiveTypes, reflect) {
    EXPECT_EQ(nvl::reflect<bool>(), nvl::type::kBool);
    EXPECT_EQ(nvl::reflect<U64>(), nvl::type::kU64);
    EXPECT_EQ(nvl::reflect<I64>(), nvl::type::kI64);
    EXPECT_EQ(nvl::reflect<F64>(), nvl::type::kF64);
}

} // namespace
