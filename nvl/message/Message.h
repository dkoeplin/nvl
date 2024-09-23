#pragma once

#include "nvl/macros/Abstract.h"
#include "nvl/reflect/Castable.h"
#include "nvl/reflect/ClassTag.h"

namespace nvl {

struct Actor;

abstract struct AbstractMessage : Castable<Message, AbstractMessage>::BaseClass {
    class_tag(AbstractMessage);
    explicit AbstractMessage(Actor src) : src(std::move(src)) {}

    Actor src;
};

struct Message final : Castable<Message, AbstractMessage> {
    using Castable::Castable;
    using Castable::get;
};

} // namespace nvl
