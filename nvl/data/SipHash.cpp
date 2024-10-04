#include "nvl/data/SipHash.h"

#include <cstring> // std::memcpy

#include "nvl/macros/Aliases.h"
#include "nvl/macros/Expand.h"
#include "nvl/math/Bitwise.h"

namespace nvl {

constexpr size_t kPacketSize = sizeof(U64);
static_assert((kPacketSize & (kPacketSize - 1)) == 0, "Size must be 2^i.");

template <int U, int F>
SipHash<U, F>::SipHash(const U64 key[2]) {
    v0 = 0x736f6d6570736575ull ^ key[0];
    v1 = 0x646f72616e646f6dull ^ key[1];
    v2 = 0x6c7967656e657261ull ^ key[0];
    v3 = 0x7465646279746573ull ^ key[1];
}

template <int U, int F>
void SipHash<U, F>::update(const char *bytes) {
    U64 packet;
    std::memcpy(&packet, bytes, sizeof(packet));

    v3 ^= packet;

    compress(U);

    v0 ^= packet;
}

template <int U, int F>
U64 SipHash<U, F>::finalize() {
    // Mix in bits to avoid leaking the key if all packets were zero.
    v2 ^= 0xFF;

    compress(F);

    return (v0 ^ v1) ^ (v2 ^ v3);
}

template <int U, int F>
void SipHash<U, F>::compress(U64 rounds) {
    for (size_t i = 0; i < rounds; ++i) {
        // ARX network: add, rotate, exclusive-or.
        v0 += v1;
        v2 += v3;
        v1 = rotate_left<13>(v1);
        v3 = rotate_left<16>(v3);
        v1 ^= v0;
        v3 ^= v2;

        v0 = rotate_left<32>(v0);

        v2 += v1;
        v0 += v3;
        v1 = rotate_left<17>(v1);
        v3 = rotate_left<21>(v3);
        v1 ^= v2;
        v3 ^= v0;

        v2 = rotate_left<32>(v2);
    }
}

template struct SipHash<2, 4>;
template struct SipHash<1, 3>;

using SipHash24 = SipHash<2, 4>;
using SipHash13 = SipHash<1, 3>;

constexpr U64 kDefaultKey[2] = {0xDEADBEEF, 0xF00DF17E};

// Copies the remaining bytes to a zero-padded buffer, sets the upper byte to
// size % 256 (always possible because this should only be called if the
// total size is not a multiple of the packet size) and updates hash state.
//
// The padding scheme is essentially from SipHash, but permuted for the
// convenience of AVX-2 masked loads. This function must use the same layout so
// that the vector and scalar HighwayTreeHash have the same result.
//
// "remaining_size" is the number of accessible/remaining bytes
// (size % kPacketSize).
//
// Primary template; the specialization for AVX-2 is faster. Intended as an
// implementation detail, do not call directly.
template <class State>
void padded_update(const U64 size, const char *remaining_bytes, const U64 remaining_size, State *state) {
    char final_packet[kPacketSize] = {0};

    // This layout matches the AVX-2 specialization in highway_tree_hash.h.
    uint32_t packet4 = static_cast<uint32_t>(size) << 24;

    const size_t remainder_mod4 = remaining_size & 3;
    if (remainder_mod4 != 0) {
        const char *final_bytes = remaining_bytes + remaining_size - remainder_mod4;
        packet4 += static_cast<uint32_t>(final_bytes[0]);
        const U64 idx1 = remainder_mod4 >> 1;
        const U64 idx2 = remainder_mod4 - 1;
        packet4 += static_cast<uint32_t>(final_bytes[idx1]) << 8;
        packet4 += static_cast<uint32_t>(final_bytes[idx2]) << 16;
    }

    std::memcpy(final_packet, remaining_bytes, remaining_size - remainder_mod4);
    std::memcpy(final_packet + kPacketSize - 4, &packet4, sizeof(packet4));

    state->update(final_packet);
}

// Updates hash state for every whole packet, and once more for the final padded packet.
template <int U, int F>
void update_state(const char *bytes, const U64 size, SipHash<U, F> *state) {
    // Feed entire packets.
    const U64 remainder = size & (kPacketSize - 1);
    const U64 truncated_size = size - remainder;
    for (U64 i = 0; i < truncated_size; i += kPacketSize) {
        state->update(bytes + i);
    }

    padded_update(size, bytes + truncated_size, remainder, state);
}

// Computes a hash of a byte array using the given hash State class.
//
// Example: const SipHash24::Key key = { 1, 2 }; char data[4];
// ComputeHash<SipHash24>(key, data, sizeof(data));
//
// This function avoids duplicating Update/Finalize in every call site.
// Callers wanting to combine multiple hashes should repeatedly update_state()
// and only call State::Finalize once.
template <class State>
pure expand U64 compute_hash(const U64 key[2], const char *bytes, const U64 size) {
    State state(key);
    update_state(bytes, size, &state);
    return state.finalize();
}

// "key" is a secret 128-bit key unknown to attackers.
// "bytes" is the data to hash; ceil(size / 8) * 8 bytes are read.
// Returns a 64-bit hash of the given data bytes, which are swapped on
// big-endian CPUs so the return value is the same as on little-endian CPUs.
pure expand U64 sip_hash24(const U64 key[2], const char *bytes, const U64 size) {
    return compute_hash<SipHash24>(key, bytes, size);
}

// Round-reduced SipHash version (1 update and 3 finalization rounds).
pure expand U64 sip_hash13(const U64 key[2], const char *bytes, const U64 size) {
    return compute_hash<SipHash13>(key, bytes, size);
}

U64 sip_hash24(const char *bytes, const U64 size) { return sip_hash24(kDefaultKey, bytes, size); }
U64 sip_hash13(const char *bytes, const U64 size) { return sip_hash13(kDefaultKey, bytes, size); }

} // namespace nvl
