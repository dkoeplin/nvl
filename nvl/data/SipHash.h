#pragma once

#include <functional>

#include "nvl/data/Range.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/Expand.h"
#include "nvl/macros/Pure.h"

namespace nvl {

// Implementation adapted from:
// https://github.com/google/highwayhash/blob/master/highwayhash/sip_hash.h
// https://github.com/google/highwayhash/blob/master/highwayhash/state_helpers.h
template <int kUpdateIters, int kFinalizeIters>
struct SipHash {
    explicit SipHash(const U64 key[2]);
    void update(const char *bytes);
    U64 finalize();

private:
    void compress(U64 rounds);
    U64 v0;
    U64 v1;
    U64 v2;
    U64 v3;
};

template <int U, int F>
void update_state(const char *bytes, const U64 size, SipHash<U, F> *state);

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

template <typename Value, typename Hash = std::hash<Value>>
pure U64 sip_hash(const Range<Value> &range) {
    const Hash hasher;
    static constexpr U64 key[2] = {0xDEADBEEF, 0xF00DF17E};
    SipHash<1, 3> state(key);
    for (const auto &value : range) {
        U64 hash = hasher(value);
        update_state(reinterpret_cast<const char *>(&hash), sizeof(U64), &state);
    }
    return state.finalize();
}

} // namespace nvl
