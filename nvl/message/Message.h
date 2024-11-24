#pragma once

#include "nvl/macros/Abstract.h"
#include "nvl/reflect/Castable.h"
#include "nvl/reflect/ClassTag.h"

namespace nvl {

struct AbstractActor;
struct Actor;
struct Message;

abstract class AbstractMessage : Castable<Message, AbstractMessage, std::shared_ptr<AbstractMessage>>::BaseClass {
public:
    class_tag(AbstractMessage);
    explicit AbstractMessage(AbstractActor *src) : src_(src) {}

    pure Actor src() const;

protected:
    AbstractActor *src_;
};

struct Message final : Castable<Message, AbstractMessage, std::shared_ptr<AbstractMessage>> {
    using Castable::Castable;
    using Castable::get;
};

} // namespace nvl
