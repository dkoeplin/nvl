#pragma once

#include "nvl/macros/Expand.h"
#include "nvl/macros/Pure.h"
#include "nvl/reflect/ClassTag.h"

namespace nvl {

template <typename B, typename A>
    requires HasClassTag<A> && HasClassTag<B>
pure expand constexpr bool isa(A *a) {
    return a != nullptr && ClassTag::get(a) <= ClassTag::get<B>();
}

template <typename B, typename A>
    requires HasClassTag<A> && HasClassTag<B>
pure expand constexpr B *dyn_cast(A *a) {
    return isa<B>(a) ? static_cast<B *>(a) : nullptr;
}

template <typename B, typename A>
    requires HasClassTag<A> && HasClassTag<B>
pure expand constexpr const B *dyn_cast(const A *a) {
    return isa<B>(a) ? static_cast<const B *>(a) : nullptr;
}

} // namespace nvl