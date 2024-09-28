#pragma once

#include "nvl/data/SipHash.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/Pure.h"

namespace nvl {

/**
 * @struct PointerHash
 * @brief Hashes dereference-able types by raw pointer.
 */
template <typename Ptr>
struct PointerHash {
    pure U64 operator()(const Ptr &a) const noexcept { return sip_hash(&*a); }
};

} // namespace nvl
