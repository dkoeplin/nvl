#pragma once

#include <memory>

#include "nvl/geo/Box.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/Pure.h"

namespace nvl {

class Draw;

template <U64 N>
class Actor {
public:
    class Impl;

    template <typename T, typename... Args>
    static Actor get(Args &&...args) {
        return Actor(std::make_unique<T>(std::forward<Args>(args)...));
    }

    void tick();
    void draw(Draw &draw) const;

    pure bool alive() const;
    pure Box<N> bbox() const;

    pure bool operator==(const Actor &rhs) const { return impl_.get() == rhs.impl_.get(); }
    pure bool operator!=(const Actor &rhs) const { return impl_.get() != rhs.impl_.get(); }

private:
    explicit Actor(std::unique_ptr<Impl> inst) : impl_(std::move(inst)) {}
    std::unique_ptr<Impl> impl_;
};

template <U64 N>
class Actor<N>::Impl {
public:
    virtual ~Impl() = default;

    virtual void tick() = 0;
    virtual void draw(Draw &draw) const = 0;

    pure virtual Box<N> bbox() const = 0;

    pure bool alive() const { return alive_; }

protected:
    bool alive_ = true;
};

template <U64 N>
void Actor<N>::tick() {
    impl_->tick();
}

template <U64 N>
void Actor<N>::draw(Draw &draw) const {
    impl_->draw(draw);
}

template <U64 N>
bool Actor<N>::alive() const {
    return impl_->alive();
}

template <U64 N>
Box<N> Actor<N>::bbox() const {
    return impl_->bbox();
}

} // namespace nvl
