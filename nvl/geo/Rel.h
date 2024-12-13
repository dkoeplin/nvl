#pragma once

#include "nvl/data/PointerHash.h"
#include "nvl/data/Ref.h"
#include "nvl/geo/HasBBox.h"
#include "nvl/geo/Volume.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/Pure.h"

namespace nvl {

/**
 * @class Rel
 * @brief Views a Value at an offset within an N-dimensional space.
 *
 * Holds a reference to the value.
 *
 * @tparam Value Value type being stored.
 */
template <typename Value>
class Rel : public Ref<Value> {
public:
    using Ref<Value>::Ref;

    template <U64 N>
        requires trait::HasBBox<Value>
    pure Box<N> bbox(const Pos<N> &offset) const {
        return this->ptr_->bbox() + offset;
    }
};

} // namespace nvl

template <typename Value>
struct std::hash<nvl::Rel<Value>> : nvl::PointerHash<nvl::Rel<Value>, Value> {};
