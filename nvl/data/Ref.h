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
    implicit Ref(Value &value) : ptr(&value) {}

    explicit operator bool() { return ptr != nullptr; }

    pure Value &operator*() { return *ptr; }
    pure const Value &operator*() const { return *ptr; }

    pure Value *operator->() { return ptr; }
    pure const Value *operator->() const { return ptr; }

    pure Value &raw() { return *ptr; }
    pure const Value &raw() const { return *ptr; }

    pure Value *pointer() { return ptr; }
    pure const Value *pointer() const { return ptr; }

    pure bool operator==(const Ref &rhs) const { return *ptr == *rhs.ptr; }
    pure bool operator!=(const Ref &rhs) const { return *ptr != *rhs.ptr; }

private:
    Value *ptr = nullptr;
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
