#pragma once

#include "nvl/macros/Aliases.h"
#include "nvl/macros/Pure.h"

namespace nvl {

/**
 * @struct PointerHash
 * @brief Hashes dereference-able types by raw pointer.
 */
template <typename Ptr, typename Type>
struct PointerHash {
    pure U64 operator()(const Ptr &a) const noexcept { return hash(&*a); }
    std::hash<const Type *> hash; // Using std::hash is a bit faster than sip_hash here
};

} // namespace nvl
