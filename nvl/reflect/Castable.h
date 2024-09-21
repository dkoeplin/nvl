#pragma once

#include <memory>

#include "nvl/macros/Expand.h"
#include "nvl/macros/Pure.h"
#include "nvl/reflect/Casting.h"

namespace nvl {

template <typename T, typename Ptr = std::shared_ptr<T>>
class Castable {
public:
    virtual ~Castable() = default;

    template <typename B>
    pure expand const B *dyn_cast() const {
        return nvl::dyn_cast<B>(_ptr());
    }

    template <typename B>
    pure expand B *dyn_cast() {
        return nvl::dyn_cast<B>(_ptr());
    }

    template <typename B>
    pure expand bool isa() const {
        return nvl::isa<B>(_ptr());
    }

protected:
    Castable() = default;
    explicit Castable(Ptr ptr) : impl_(ptr) {}

    pure expand const T *_ptr() const { return impl_.get(); }
    pure expand T *_ptr() { return impl_.get(); }
    Ptr impl_ = nullptr;
};

} // namespace nvl
