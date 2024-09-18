#pragma once

#include <memory>
#include <string>

#include "nvl/draw/Color.h"
#include "nvl/macros/Pure.h"

namespace nvl {

class Material {
public:
    struct Impl;

    template <typename T, typename... Args>
    static Material get(Args &&...args) {
        return Material(std::make_unique<T>(std::forward<Args>(args)...));
    }

    pure std::string name() const;
    pure Color color() const;
    pure bool falls() const;
    pure bool liquid() const;
    pure U64 durability() const;

private:
    std::shared_ptr<Impl> impl_;
};

struct Material::Impl {
    std::string name;
    Color color;
    bool falls;
    bool liquid;
    U64 durability;
};

} // namespace nvl
