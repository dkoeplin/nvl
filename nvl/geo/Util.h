#pragma once

#ifndef NVL_UTIL_H
#define NVL_UTIL_H

#include "nvl/data/Maybe.h"
#include "nvl/data/Ref.h"
#include "nvl/geo/Tuple.h"
#include "nvl/geo/Volume.h"

namespace nvl {

template <U64 N, typename T, typename Item>
    requires trait::HasBBox<N, I64, Item>
pure Volume<N, T> bounding_box(const Range<Ref<Item>> &items) {
    Maybe<Volume<N, T>> result;
    for (auto item : items) {
        result = !result ? item->bbox() : bounding_box(item->bbox(), *result);
    }
    return result.value_or(Volume<N, T>{Tuple<N, T>::zero, Tuple<N, T>::zero});
}

} // namespace nvl

#endif // NVL_UTIL_H
