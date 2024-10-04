#pragma once

#include <string_view>

#include "nvl/geo/Pos.h"
#include "nvl/macros/Pure.h"

namespace nvl {

struct Color;

template <U64 N>
class Box;

class Window {
public:
    class Offset {
    public:
        explicit Offset(Window &parent, const Pos<2> &offset);
        ~Offset();

    private:
        Window &parent_;
        Maybe<Pos<2>> prev_offset_;
    };

    explicit Window(std::string_view title, Pos<2> shape);
    ~Window();

    void draw();

    void line_rectangle(const Color &color, const Box<2> &box);
    void fill_rectangle(const Color &color, const Box<2> &box);

    pure bool should_close() const;

private:
    friend class Offset;

    Maybe<Pos<2>> offset_ = None;
};

} // namespace nvl
