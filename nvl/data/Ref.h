#pragma once

#include "nvl/macros/Aliases.h"
#include "nvl/macros/Expand.h"
#include "nvl/macros/Implicit.h"
#include "nvl/macros/Pure.h"

namespace nvl {

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

    pure bool operator==(const Ref &rhs) const { return *ptr_ == *rhs.ptr_; }
    pure bool operator!=(const Ref &rhs) const { return *ptr_ != *rhs.ptr_; }

private:
    Value *ptr_ = nullptr;
};

template <typename Value>
expand std::ostream &operator<<(std::ostream &os, const Ref<Value> &ref) {
    return os << ref.raw();
}

} // namespace nvl

template <typename Value>
struct std::hash<nvl::Ref<Value>> {
    pure U64 operator()(const nvl::Ref<Value> &a) const { return std::hash<Value>()(a.raw()); }
};
