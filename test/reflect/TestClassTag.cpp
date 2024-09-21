#include <gtest/gtest.h>

#include <utility>

#include "nvl/reflect/Casting.h"
#include "nvl/reflect/ClassTag.h"

namespace {

using nvl::ClassTag;

struct Parent {
    class_tag(Parent);
    virtual ~Parent() = default;
};

struct Child final : Parent {
    class_tag(Child, Parent);
    explicit Child(std::string name) : name(std::move(name)) {}
    pure std::string get_name() const { return name; }
    std::string name;
};

TEST(TestClassTag, subclass) {
    EXPECT_TRUE(ClassTag::get<Child>() <= ClassTag::get<Parent>());
    EXPECT_TRUE(ClassTag::get<Child>() < ClassTag::get<Parent>());
    EXPECT_TRUE(ClassTag::get<Child>() <= ClassTag::get<Child>());
    EXPECT_TRUE(ClassTag::get<Child>() == ClassTag::get<Child>());
    EXPECT_TRUE(ClassTag::get<Parent>() >= ClassTag::get<Child>());
    EXPECT_TRUE(ClassTag::get<Parent>() > ClassTag::get<Child>());
    EXPECT_TRUE(ClassTag::get<Parent>() != ClassTag::get<Child>());

    std::unique_ptr<Parent> inst = std::make_unique<Child>("BLARGH");
    Parent *a = inst.get();
    EXPECT_TRUE(ClassTag::get(a) <= ClassTag::get<Parent>());
    EXPECT_TRUE(ClassTag::get(a) < ClassTag::get<Parent>());
    EXPECT_TRUE(ClassTag::get(a) != ClassTag::get<Parent>());
    EXPECT_TRUE(ClassTag::get(a) <= ClassTag::get<Child>());
    EXPECT_TRUE(ClassTag::get(a) == ClassTag::get<Child>());

    EXPECT_TRUE(nvl::isa<Child>(a));
    Child *b = nvl::dyn_cast<Child>(a);
    ASSERT_TRUE(b != nullptr);
    EXPECT_EQ(b->get_name(), "BLARGH");
}

struct A {
    class_tag(A);
    virtual ~A() = default;
};
struct B {
    class_tag(B);
    virtual ~B() = default;
};
struct C final : A, B {
    class_tag(C, A, B);
    explicit C(std::string x) : x(std::move(x)) {}
    std::string x;
};

TEST(TestClassTag, multiple_inheritance) {
    EXPECT_TRUE(ClassTag::get<C>() <= ClassTag::get<A>());
    EXPECT_TRUE(ClassTag::get<C>() <= ClassTag::get<B>());
    EXPECT_TRUE(ClassTag::get<C>() <= ClassTag::get<C>());
    std::unique_ptr<A> inst = std::make_unique<C>("TEST");
    A *a = inst.get();
    EXPECT_TRUE(ClassTag::get(a) <= ClassTag::get<A>());
    EXPECT_TRUE(ClassTag::get(a) < ClassTag::get<A>());
    EXPECT_TRUE(ClassTag::get(a) != ClassTag::get<A>());
    EXPECT_TRUE(ClassTag::get(a) <= ClassTag::get<B>());
    EXPECT_TRUE(ClassTag::get(a) < ClassTag::get<B>());
    EXPECT_TRUE(ClassTag::get(a) != ClassTag::get<B>());
    EXPECT_TRUE(ClassTag::get(a) <= ClassTag::get<C>());
    EXPECT_TRUE(ClassTag::get(a) == ClassTag::get<C>());

    C *c = nvl::dyn_cast<C>(a);
    EXPECT_TRUE(c != nullptr);
    EXPECT_EQ(c->x, "TEST");
}

struct Foo {
    class_tag(Foo);
    virtual ~Foo() = default;
};

template <typename T>
struct Bar final : Foo {
    class_tag(Bar<T>, Foo);
    explicit Bar(T value) : value(value) {}
    T value;
};

TEST(TestClassTag, templated_types) {
    EXPECT_TRUE(ClassTag::get<Bar<I64>>() == ClassTag::get<Bar<I64>>());
    EXPECT_TRUE(ClassTag::get<Bar<U64>>() != ClassTag::get<Bar<I64>>());
    // This will just be Bar<T> for both. Sad days.
    std::cout << "Bar<U64>: " << ClassTag::get<Bar<U64>>() << std::endl;
    std::cout << "Bar<I64>: " << ClassTag::get<Bar<I64>>() << std::endl;

    std::unique_ptr<Foo> inst = std::make_unique<Bar<U64>>(32);
    Foo *foo = inst.get();

    EXPECT_TRUE(ClassTag::get(foo) <= ClassTag::get<Foo>());
    EXPECT_TRUE(ClassTag::get(foo) < ClassTag::get<Foo>());
    EXPECT_TRUE(ClassTag::get(foo) != ClassTag::get<Foo>());
    EXPECT_FALSE(ClassTag::get(foo) <= ClassTag::get<Bar<I64>>());
    EXPECT_FALSE(ClassTag::get(foo) < ClassTag::get<Bar<I64>>());
    EXPECT_FALSE(ClassTag::get(foo) == ClassTag::get<Bar<I64>>());
    EXPECT_TRUE(ClassTag::get(foo) <= ClassTag::get<Bar<U64>>());
    EXPECT_FALSE(ClassTag::get(foo) < ClassTag::get<Bar<U64>>());
    EXPECT_TRUE(ClassTag::get(foo) == ClassTag::get<Bar<U64>>());

    Bar<I64> *bar_wrong = nvl::dyn_cast<Bar<I64>>(foo);
    EXPECT_TRUE(bar_wrong == nullptr);

    Bar<U64> *bar = nvl::dyn_cast<Bar<U64>>(foo);
    EXPECT_TRUE(bar != nullptr);
    EXPECT_EQ(bar->value, 32);
}

} // namespace