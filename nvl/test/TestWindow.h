#pragma once

#include "nvl/data/Tensor.h"
#include "nvl/draw/Color.h"
#include "nvl/ui/Window.h"

namespace nvl::test {

class TestWindow final : public Window {
public:
    explicit TestWindow(std::string_view title, Pos<2> shape)
        : Window(title, shape), title_(title), tensor_(shape, Color::kWhite) {}
    void draw() override {}
    void tick() override {}
    void line_rectangle(const Color &color, const Box<2> &box) override;
    void fill_rectangle(const Color &color, const Box<2> &box) override;
    void text(const Color &color, const Pos<2> &pos, I64 font_size, std::string_view text) override {}
    void centered_text(const Color &color, const Pos<2> &pos, I64 font_size, std::string_view text) override {}

    void set_view_offset(const Maybe<Pos<2>> &offset) override;
    pure bool should_close() const override { return false; }
    pure I64 height() const override { return tensor_.shape()[1]; }
    pure I64 width() const override { return tensor_.shape()[0]; }
    pure I64 fps() const override { return 0; }

private:
    std::string title_;
    Tensor<2, Color> tensor_;
};

} // namespace nvl::test
