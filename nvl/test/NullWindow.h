#pragma once

#include "nvl/ui/Window.h"

namespace nvl::test {

class NullWindow final : public Window {
public:
    NullWindow() : Window("null", {0, 0}) {}
    List<InputEvent> detect_events() override { return {}; }
    void line_box(const Color &, const Box<2> &) override {}
    void fill_box(const Color &, const Box<2> &) override {}
    void line_cube(const Color &, const Box<3> &) override {}
    void fill_cube(const Color &, const Box<3> &) override {}
    void line(const Color &, const Line<2> &) override {}
    void line(const Color &, const Line<3> &) override {}

    void text(const Color &, const Pos<2> &, I64, std::string_view) override {}
    void centered_text(const Color &, const Pos<2> &, I64, std::string_view) override {}
    void set_view_offset(const ViewOffset &) override {}
    void end_view_offset(const ViewOffset &) override {}
    pure bool should_close() const override { return false; }
    pure I64 height() const override { return 0; }
    pure I64 width() const override { return 0; }
    pure I64 fps() const override { return 0; }
};

} // namespace nvl::test
