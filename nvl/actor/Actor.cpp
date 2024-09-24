#include "nvl/actor/Actor.h"

#include "nvl/data/SipHash.h"

U64 std::hash<nvl::Actor>::operator()(const nvl::Actor &actor) const noexcept { return sip_hash(actor.ptr()); }
