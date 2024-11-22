#pragma once

#include "nvl/geo/Dir.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/Pure.h"

namespace nvl {

/**
 * @struct Face
 * @brief A tuple of dimension and direction.
 * Used to reference a specific surface/face of an ND volume.
 */
struct Face {
    Face() = default;
    explicit Face(const Dir dir, const U64 dim) : dir(dir), dim(dim) {}

    pure Face operator-() const { return Face(-dir, dim); }
    pure bool operator==(const Face &rhs) const { return dim == rhs.dim && dir == rhs.dir; }
    pure bool operator!=(const Face &rhs) const { return !(*this == rhs); }

    Dir dir = Dir::Pos;
    U64 dim = 0;
};

inline std::ostream &operator<<(std::ostream &os, const Face &face) { return os << face.dir << face.dim; }

} // namespace nvl
