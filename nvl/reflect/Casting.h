#pragma once

#include "nvl/reflect/ClassTag.h"

namespace nvl {

template <typename B, typename A>
    requires HasClassTag<A> && HasClassTag<B>
constexpr bool isa(A *a) {
    return ClassTag::get(a) <= ClassTag::get<B>();
}

template <typename B, typename A>
    requires HasClassTag<A> && HasClassTag<B>
constexpr B *dyn_cast(A *a) {
    return isa<B>(a) ? static_cast<B *>(a) : nullptr;
}

} // namespace nvl