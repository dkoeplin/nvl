#pragma once

#include "nvl/ui/Window.h"

namespace nvl::test {

class NullWindow final : public Window {
public:
    NullWindow() : Window("null", {0, 0}) {}
    explicit NullWindow(std::string_view title, Pos<2> shape) : Window(title, shape) {}
    void draw() override {}
    void tick() override {}
    void line_rectangle(const Color &, const Box<2> &) override {}
    void fill_rectangle(const Color &, const Box<2> &) override {}
    void text(const Color &, const Pos<2> &, I64, std::string_view) override {}
    void centered_text(const Color &, const Pos<2> &, I64, std::string_view) override {}
    void set_view_offset(const Maybe<Pos<2>> &) override {}
    pure bool should_close() const override { return false; }
    pure I64 height() const override { return 0; }
    pure I64 width() const override { return 0; }
    pure I64 fps() const override { return 0; }
};

} // namespace nvl::test
