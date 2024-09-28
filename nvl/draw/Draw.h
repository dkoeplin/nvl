#pragma once

#include "nvl/geo/Pos.h"
#include "nvl/macros/Pure.h"

namespace nvl {

class Draw {
public:
    struct Offset {
        explicit Offset(Draw &draw, const Pos<2> &offset) : draw(draw), prev_offset(draw.get_offset()) {
            draw.set_offset(offset);
        }
        ~Offset() { draw.set_offset(prev_offset); }

        Draw &draw;
        Pos<2> prev_offset;
    };

private:
    friend struct Offset;

    pure Pos<2> get_offset() const { return offset_; }
    void set_offset(const Pos<2> &offset) { offset_ = offset; }

    Pos<2> offset_;
};

} // namespace nvl
