#pragma once

#include "nvl/geo/Box.h"
#include "nvl/macros/Aliases.h"
#include "nvl/message/Message.h"

namespace nvl {

template <U64 N>
struct Hit final : AbstractMessage {
    class_tag(Hit<N>);
    Box<N> box;
    U64 strength = 0;
};

} // namespace nvl
