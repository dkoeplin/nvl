#pragma once

#include "nvl/macros/Expand.h"
#include "nvl/macros/Implicit.h"
#include "nvl/macros/Pure.h"
#include "nvl/reflect/Casting.h"

namespace nvl {

/**
 * @class CastablePtr
 * @brief Provides a thin wrapper around a raw pointer with methods for dynamic casting.
 *
 * @tparam Ref - The wrapper type
 * @tparam T - The common parent class of all subclasses
 */
template <typename Ref, typename T>
class CastablePtr {
public:
    struct BaseClass {
        virtual ~BaseClass() = default;
        // Loses const here - there's not a good way to maintain const on the returned Ref
        Ref self() const { return Ref(const_cast<T *>(static_cast<const T *>(this))); }
    };
    struct Hash {
        U64 operator()(const Ref &ref) const { return sip_hash(ref.ptr()); }
    };

    CastablePtr() = default;
    explicit CastablePtr(T *ptr) : ptr_(ptr) {}
    implicit CastablePtr(nullptr_t) : ptr_(nullptr) {}

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
    T *ptr_ = nullptr;
};

} // namespace nvl
