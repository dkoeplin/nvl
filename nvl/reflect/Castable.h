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
        const Ref &self() const { return *self_; }
        Ref &self() { return *self_; }

        Ref *self_ = nullptr;
    };
    struct Hash {
        U64 operator()(const Ref &ref) const { return sip_hash(ref.ptr()); }
    };

    template <typename R, typename... Args>
        requires std::is_same_v<Ptr, std::unique_ptr<T>> || std::is_same_v<Ptr, std::shared_ptr<T>>
    static Ref get(Args &&...args) {
        R *inst = new R(std::forward<Args>(args)...);
        Ptr ptr(inst);
        return Ref(std::move(ptr));
    }

    Castable() = default;
    explicit Castable(Ptr ptr) : ptr_(ptr) {
        if (ptr != nullptr) {
            ptr->self_ = this;
        }
    }
    implicit Castable(nullptr_t) : ptr_(nullptr) {}

    explicit operator bool() const { return ptr() != nullptr; }

    pure bool operator==(nullptr_t) const { return ptr() == nullptr; }
    pure bool operator!=(nullptr_t) const { return ptr() != nullptr; }

    T &operator*() { return *ptr_; }
    const T &operator*() const { return *ptr_; }

    T *operator->() { return ptr(); }
    const T *operator->() const { return ptr(); }

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
