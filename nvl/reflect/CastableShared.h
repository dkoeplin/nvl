#pragma once

#include <memory>

#include "nvl/macros/Expand.h"
#include "nvl/macros/Implicit.h"
#include "nvl/macros/Pure.h"
#include "nvl/reflect/Casting.h"

namespace nvl {

/**
 * @class CastableShared
 * @brief Provides a thin wrapper around a shared pointer with methods for dynamic casting.
 * @tparam Ref The wrapper type
 * @tparam T The common parent class of all subclasses
 */
template <typename Ref, typename T>
class CastableShared {
public:
    struct BaseClass {
        virtual ~BaseClass() = default;

        Ref self() const { return Ref(self_.lock()); }

        std::weak_ptr<T> self_;
    };
    struct Hash {
        U64 operator()(const Ref &ref) const { return sip_hash(ref.ptr()); }
    };

    template <typename R, typename... Args>
    static Ref get(Args &&...args) {
        return Ref(std::make_shared<R>(std::forward<Args>(args)...));
    }

    CastableShared() = default;

    explicit CastableShared(std::shared_ptr<T> ptr) : ptr_(std::move(ptr)) {
        if (ptr_) {
            ptr_->self_ = ptr_;
        }
    }

    implicit CastableShared(nullptr_t) : ptr_(nullptr) {}

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
    std::shared_ptr<T> ptr_ = nullptr;
};

} // namespace nvl
