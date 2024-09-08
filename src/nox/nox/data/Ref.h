#pragma once

#include "nox/macros/Implicit.h"
#include "nox/macros/Pure.h"

namespace nox {

template <typename Value> class Ref {
  public:
    Ref() = default;
    implicit Ref(Value &value) : ptr(&value) {}

    explicit operator bool() { return ptr != nullptr; }

    Value *operator*() { return ptr; }
    const Value *operator*() const { return ptr; }

    Value *operator->() { return ptr; }
    const Value *operator->() const { return ptr; }

    Value &raw() { return *ptr; }
    const Value &raw() const { return *ptr; }

    pure bool operator==(const Ref &rhs) const { return *ptr == *rhs.ptr; }
    pure bool operator!=(const Ref &rhs) const { return *ptr != *rhs.ptr; }

  private:
    Value *ptr = nullptr;
};

template <typename Value> std::ostream &operator<<(std::ostream &os, Ref<Value> ref) { return os << ref.raw(); }

} // namespace nox
