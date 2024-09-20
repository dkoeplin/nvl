#pragma once

#include <memory>

#include "nvl/data/List.h"
#include "nvl/data/Ref.h"
#include "nvl/macros/Abstract.h"
#include "nvl/reflect/Casting.h"
#include "nvl/reflect/ClassTag.h"

namespace nvl {

class Actor;

class Message final {
public:
    struct Impl;
    template <typename T, typename... Args>
    static Message get(Args &&...args) {
        return Message(std::shared_ptr<T>(std::forward<Args>(args)...));
    }

    template <typename T>
    pure const T *dyn_cast() const {
        return dyn_cast<T>(impl_.get());
    }
    template <typename T>
    pure T *dyn_cast() {
        return dyn_cast<T>(impl_.get());
    }

    pure Ref<Actor> src() const;
    pure List<Ref<Actor>> dst() const;

private:
    explicit Message(std::shared_ptr<Impl> inst) : impl_(std::move(inst)) {}
    std::shared_ptr<Impl> impl_;
};

abstract struct Message::Impl {
public:
    class_tag(Message::Impl);
    virtual ~Impl() = default;

    Ref<Actor> src;
    List<Ref<Actor>> dst;

protected:
    Impl() = default;
};

} // namespace nvl
