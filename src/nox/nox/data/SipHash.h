#pragma once

#include "nox/macros/Aliases.h"

namespace nox {

// U64 sip_hash24(const U64 key[2], const char *bytes, U64 size);
// U64 sip_hash13(const U64 key[2], const char *bytes, U64 size);

U64 sip_hash24(const char *bytes, U64 size);
U64 sip_hash13(const char *bytes, U64 size);

// Basic hash for anything that can be reinterpreted as an array of bytes.
// Note that this isn't portable - it may give different results on different target machines.
template <typename Value> U64 sip_hash(const Value &value) {
    return sip_hash13(reinterpret_cast<const char *>(&value), sizeof(value));
}

} // namespace nox
