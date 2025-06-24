#pragma once

#include <string>

#include "nvl/macros/Abstract.h"
#include "nvl/reflect/CastableShared.h"
#include "nvl/reflect/ClassTag.h"

namespace nvl {

struct AbstractActor;
struct Actor;
struct Message;

abstract class AbstractMessage : public CastableShared<Message, AbstractMessage>::BaseClass {
public:
    class_tag(AbstractMessage);
    explicit AbstractMessage(AbstractActor *src) : src_(src) {}

    pure Actor src() const;
    pure virtual std::string to_string() const = 0;

protected:
    AbstractActor *src_;
};

struct Message final : CastableShared<Message, AbstractMessage> {
    using CastableShared::CastableShared;
    using CastableShared::get;
};

inline std::ostream &operator<<(std::ostream &os, const Message &message) { return os << message->to_string(); }

} // namespace nvl
