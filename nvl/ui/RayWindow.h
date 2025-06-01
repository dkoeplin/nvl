#pragma once

#include "nvl/ui/Window.h"

namespace nvl {

class RayWindow : public Window {
public:
    explicit RayWindow(const std::string &title, Pos<2> shape);
    ~RayWindow() override;
    List<InputEvent> detect_events() override;

    void predraw() override;
    void postdraw() override;

    void line_box(const Color &color, const Box<2> &box) override;
    void fill_box(const Color &color, const Box<2> &box) override;

    void line_cube(const Color &color, const Box<3> &cube) override;
    void fill_cube(const Color &color, const Box<3> &cube) override;

    void line(const Color &color, const Line<2> &line) override;
    void line(const Color &color, const Line<3> &line) override;

    void text(const Color &color, const Pos<2> &pos, I64 font_size, std::string_view text) override;
    void centered_text(const Color &color, const Pos<2> &pos, I64 font_size, std::string_view text) override;

    void set_view_offset(const ViewOffset &view) override;
    void end_view_offset(const ViewOffset &view) override;

    void set_mouse_mode(MouseMode mode) override;
    pure bool should_close() const override;
    pure I64 height() const override;
    pure I64 width() const override;
    pure I64 fps() const override;

    void set_target_fps(U64 fps) const override;

private:
    F64 scale_ = 1.0;
};

} // namespace nvl
