#pragma once

#include "nvl/ui/Window.h"

namespace nvl {

class RayWindow : public Window {
public:
    explicit RayWindow(const std::string &title, Pos<2> shape);
    ~RayWindow() override;
    void draw() override;
    void feed() override;
    void line_box(const Color &color, const Box<2> &box) override;
    void fill_box(const Color &color, const Box<2> &box) override;
    void line_cube(const Color &color, const Box<3> &cube) override;
    void fill_cube(const Color &color, const Box<3> &cube) override;

    void text(const Color &color, const Pos<2> &pos, I64 font_size, std::string_view text) override;
    void centered_text(const Color &color, const Pos<2> &pos, I64 font_size, std::string_view text) override;

    void set_view_offset(const ViewOffset &offset) override;
    void end_view_offset(const ViewOffset &offset) override;

    void set_mouse_mode(MouseMode mode) override;
    pure bool should_close() const override;
    pure I64 height() const override;
    pure I64 width() const override;
    pure I64 fps() const override;

protected:
    void tick() override {}
};

} // namespace nvl
