#include <gtest/gtest.h>

#include <utility>

#include "nvl/reflect/Casting.h"
#include "nvl/reflect/ClassTag.h"

namespace {

struct A {
    dyn_tag(A);
    virtual ~A() = default;
};

struct B : A {
    dyn_tag(B, A);
    explicit B(std::string name) : name(std::move(name)) {}
    pure std::string get_name() const { return name; }
    std::string name;
};

TEST(TestCasting, dyn_cast) {
    std::unique_ptr<A> inst = std::make_unique<B>("BLARGH");
    A *a = inst.get();
    EXPECT_TRUE(nvl::isa<B>(a));
    B *b = nvl::dyn_cast<B>(a);
    ASSERT_TRUE(b != nullptr);
    EXPECT_EQ(b->get_name(), "BLARGH");
}

} // namespace