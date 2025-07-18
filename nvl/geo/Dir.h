#pragma once

#include <ostream>

#include "nvl/macros/Aliases.h"
#include "nvl/macros/Implicit.h"
#include "nvl/macros/Pure.h"

namespace nvl {

/**
 * @class Dir
 * @brief Denotes positive or negative direction in an associated dimension.
 */
class Dir {
public:
    static const Dir Pos;
    static const Dir Neg;
    static const Dir list[2];

    Dir() = default;
    pure implicit operator I64() const { return value; }

    pure constexpr Dir operator-() const { return Dir(-value); }
    pure bool operator==(const Dir &rhs) const { return value == rhs.value; }
    pure bool operator!=(const Dir &rhs) const { return value != rhs.value; }
    pure std::string to_string() const { return value == -1 ? "-" : "+"; }

private:
    explicit constexpr Dir(const I64 v) : value(v) {}
    I64 value = -1;
};

constexpr Dir Dir::Pos = Dir(1);
constexpr Dir Dir::Neg = Dir(-1);
constexpr Dir Dir::list[2] = {Pos, Neg};

inline std::ostream &operator<<(std::ostream &os, const Dir &dir) { return os << (dir == Dir::Neg ? "-" : "+"); }

} // namespace nvl
