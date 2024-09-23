#pragma once

#include "nvl/macros/Expand.h"
#include "nvl/macros/Implicit.h"
#include "nvl/macros/Pure.h"
#include "nvl/reflect/Casting.h"

namespace nvl {

template <typename Ref, typename T, typename Ptr = T *>
class Castable {
public:
    struct BaseClass {
        virtual ~BaseClass() = default;
        T self() const { return T(Ptr(this)); }
    };

    template <typename R, typename... Args>
    static Ref get(Args &&...args) {
        return Ref(Ptr(new R(std::forward<Args>(args)...)));
    }

    Castable() = default;
    explicit Castable(Ptr ptr) : ptr_(ptr) {}
    implicit Castable(nullptr_t) : ptr_(nullptr) {}
    virtual ~Castable() = default;

    T &operator*() { return *ptr_; }
    const T &operator*() const { return *ptr_; }

    T *operator->() { return ptr_; }
    const T *operator->() const { return ptr_; }

    template <typename B>
    pure expand const B *dyn_cast() const {
        return nvl::dyn_cast<B>(ptr());
    }

    template <typename B>
    pure expand B *dyn_cast() {
        return nvl::dyn_cast<B>(ptr());
    }

    template <typename B>
    pure expand bool isa() const {
        return nvl::isa<B>(ptr());
    }

    pure expand T *ptr() { return &*ptr_; }
    pure expand const T *ptr() const { return &*ptr_; }

protected:
    Ptr ptr_ = nullptr;
};

} // namespace nvl
