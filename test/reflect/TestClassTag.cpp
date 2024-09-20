#include <gtest/gtest.h>

#include <utility>

#include "nvl/reflect/ClassTag.h"

namespace {

using nvl::ClassTag;

struct A {
    class_tag(A);
    virtual ~A() = default;
};

struct B : A {
    class_tag(B, A);
    explicit B(std::string name) : name(std::move(name)) {}
    pure std::string get_name() const { return name; }
    std::string name;
};

TEST(TestClassTag, subclass) {
    EXPECT_TRUE(ClassTag::get<B>() <= ClassTag::get<A>());
    EXPECT_TRUE(ClassTag::get<B>() < ClassTag::get<A>());
    EXPECT_TRUE(ClassTag::get<B>() <= ClassTag::get<B>());
    EXPECT_TRUE(ClassTag::get<B>() == ClassTag::get<B>());
    EXPECT_TRUE(ClassTag::get<A>() >= ClassTag::get<B>());
    EXPECT_TRUE(ClassTag::get<A>() > ClassTag::get<B>());
    EXPECT_TRUE(ClassTag::get<A>() != ClassTag::get<B>());

    std::unique_ptr<A> inst = std::make_unique<B>("BLARGH");
    A *a = inst.get();
    EXPECT_TRUE(ClassTag::get(a) <= ClassTag::get<A>());
    EXPECT_TRUE(ClassTag::get(a) < ClassTag::get<A>());
    EXPECT_TRUE(ClassTag::get(a) != ClassTag::get<A>());
    EXPECT_TRUE(ClassTag::get(a) <= ClassTag::get<B>());
    EXPECT_TRUE(ClassTag::get(a) == ClassTag::get<B>());
}

} // namespace