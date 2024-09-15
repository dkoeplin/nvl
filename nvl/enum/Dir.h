#pragma once

#include "nvl/macros/Aliases.h"
#include "nvl/macros/Pure.h"

namespace nvl {

class Dir {
public:
    static const Dir Pos;
    static const Dir Neg;
    static const Dir list[2];

    Dir() = default;

    pure constexpr Dir operator-() const { return Dir(-value); }
    pure bool operator==(const Dir &rhs) const { return value == rhs.value; }
    pure bool operator!=(const Dir &rhs) const { return value != rhs.value; }

private:
    explicit constexpr Dir(const I64 v) : value(v) {}
    I64 value = -1;
};

constexpr Dir Dir::Pos = Dir(1);
constexpr Dir Dir::Neg = Dir(-1);
constexpr Dir Dir::list[2] = {Pos, Neg};

} // namespace nvl
