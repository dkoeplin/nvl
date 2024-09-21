#pragma once

#include "nvl/macros/Aliases.h"
#include "nvl/macros/Pure.h"
#include "nvl/macros/Unreachable.h"
#include "nvl/reflect/ClassTag.h"

namespace nvl {

namespace type {
constexpr auto kBool = ClassTag("bool");
constexpr auto kU64 = ClassTag("U64");
constexpr auto kI64 = ClassTag("I64");
constexpr auto kF64 = ClassTag("F64");
} // namespace type

template <>
pure inline const ClassTag &reflect<bool>() {
    return type::kBool;
}
template <>
pure inline const ClassTag &reflect<U64>() {
    return type::kU64;
}
template <>
pure inline const ClassTag &reflect<I64>() {
    return type::kI64;
}
template <>
pure inline const ClassTag &reflect<F64>() {
    return type::kF64;
}

} // namespace nvl
