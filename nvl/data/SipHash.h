#pragma once

#include "nvl/macros/Aliases.h"
#include "nvl/macros/Expand.h"
#include "nvl/macros/Pure.h"

namespace nvl {

// U64 sip_hash24(const U64 key[2], const char *bytes, U64 size);
// U64 sip_hash13(const U64 key[2], const char *bytes, U64 size);

pure U64 sip_hash24(const char *bytes, U64 size);
pure U64 sip_hash13(const char *bytes, U64 size);

/// Returns a hash for anything that can be reinterpreted as an array of bytes.
/// Uses SipHash13 as the default hashing algorithm:
///   SipHash Paper: https://www.131002.net/siphash/siphash.pdf
/// Implementation adapted from:
///   https://github.com/google/highwayhash/blob/master/highwayhash/sip_hash.h
///   https://github.com/google/highwayhash/blob/master/highwayhash/state_helpers.h
/// Note that the adapted implementation isn't portable - it may give different results on different target machines.
template <typename Value>
pure expand U64 sip_hash(const Value &value) {
    return sip_hash13(reinterpret_cast<const char *>(&value), sizeof(value));
}

} // namespace nvl
