#pragma once

#include "nvl/macros/Aliases.h"
#include "nvl/macros/Pure.h"
#include "nvl/macros/Unreachable.h"

namespace nvl {

enum class PrimitiveType { kBool, kU64, kI64, kF64 };

template <typename T>
pure PrimitiveType reflect() {
    UNREACHABLE();
}
template <>
pure inline PrimitiveType reflect<bool>() {
    return PrimitiveType::kBool;
}
template <>
pure inline PrimitiveType reflect<U64>() {
    return PrimitiveType::kU64;
}
template <>
pure inline PrimitiveType reflect<I64>() {
    return PrimitiveType::kI64;
}
template <>
pure inline PrimitiveType reflect<F64>() {
    return PrimitiveType::kF64;
}

} // namespace nvl
