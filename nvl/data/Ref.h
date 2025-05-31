#pragma once

#include "nvl/data/HasEquality.h"
#include "nvl/data/PointerHash.h"
#include "nvl/io/HasPrint.h"
#include "nvl/macros/Expand.h"
#include "nvl/macros/Implicit.h"
#include "nvl/macros/Pure.h"
#include "nvl/macros/ReturnIf.h"

namespace nvl {

/**
 * @class Ref
 * @brief A non-null reference to an object (internally kept as a pointer).
 * Primary use case is to store references in collections.
 *
 * @tparam Value - The value type being referenced.
 */
template <typename Value>
class Ref {
public:
    Ref() = default;
    implicit Ref(Value &value) : ptr_(&value) {}
    explicit Ref(Value *value) : ptr_(value) {}

    explicit operator bool() { return ptr_ != nullptr; }

    pure Value &operator*() { return *ptr_; }
    pure const Value &operator*() const { return *ptr_; }

    pure Value *operator->() { return ptr_; }
    pure const Value *operator->() const { return ptr_; }

    pure Value &raw() { return *ptr_; }
    pure const Value &raw() const { return *ptr_; }

    pure Value *ptr() { return ptr_; }
    pure const Value *ptr() const { return ptr_; }

    pure bool operator==(const Ref &rhs) const {
        return_if(ptr_ == rhs.ptr_, true);
        if constexpr (trait::HasEquality<Value>) {
            return_if(*ptr_ == *rhs.ptr_, true);
        }
        return false;
    }
    pure bool operator!=(const Ref &rhs) const { return !(*this == rhs); }

protected:
    Value *ptr_ = nullptr;
};

template <typename Value>
expand std::ostream &operator<<(std::ostream &os, const Ref<Value> &ref) {
    if constexpr (trait::HasPrint<Value>) {
        return os << ref.raw();
    } else {
        return os << ref.ptr();
    }
}

} // namespace nvl

/// By default, Ref is hashed like a raw pointer.
template <typename Value>
struct std::hash<nvl::Ref<Value>> : nvl::PointerHash<nvl::Ref<Value>, Value> {};
