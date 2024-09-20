#pragma once

namespace nvl {

class Draw {
public:
    template <U64 N>
    struct Offset {
        explicit Offset(Draw &draw, const Pos<N> &offset) : draw(draw), prev_offset(draw.get_offset<N>()) {
            draw.set_offset(offset);
        }
        ~Offset() { draw.set_offset(prev_offset); }

        Draw &draw;
        Pos<N> prev_offset;
    };

private:
    template <U64 N>
    friend struct Offset;

    // TODO: Stubs
    template <U64 N>
    pure Pos<N> get_offset() const {
        return Pos<N>::fill(1);
    }

    template <U64 N>
    void set_offset(const Pos<N> &) {}
};

} // namespace nvl
