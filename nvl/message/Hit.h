#pragma once

#include "nvl/geo/Volume.h"
#include "nvl/macros/Aliases.h"
#include "nvl/message/Message.h"

namespace nvl {

template <U64 N>
struct Hit final : AbstractMessage {
    class_tag(Hit<N>, AbstractMessage);
    explicit Hit(AbstractActor *src, const Box<N> box, const I64 strength)
        : AbstractMessage(src), box(box), strength(strength) {}

    pure std::string to_string() const override {
        return "Hit(" + box.to_string() + ", " + std::to_string(strength) + ")";
    }

    Box<N> box;
    I64 strength = 0;
};

} // namespace nvl
