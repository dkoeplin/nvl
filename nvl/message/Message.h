#pragma once

#include <utility>

#include "nvl/data/List.h"
#include "nvl/data/Ref.h"
#include "nvl/macros/Abstract.h"
#include "nvl/reflect/Castable.h"
#include "nvl/reflect/ClassTag.h"

namespace nvl {

class Actor;

abstract struct AbstractMessage {
    class_tag(AbstractMessage);
    virtual ~AbstractMessage() = default;

    Ref<Actor> src;
    List<Ref<Actor>> dst;
};

class Message final : public Castable<AbstractMessage> {
public:
    template <typename T, typename... Args>
    static Message get(Args &&...args) {
        return Message(std::shared_ptr<T>(std::forward<Args>(args)...));
    }

    pure Ref<Actor> src() const { return impl_->src; }
    pure List<Ref<Actor>> dst() const { return impl_->dst; }

private:
    using Castable::Castable;
};

} // namespace nvl
