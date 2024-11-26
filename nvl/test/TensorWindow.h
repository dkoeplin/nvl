#pragma once

#include "nvl/data/Tensor.h"
#include "nvl/ui/Color.h"
#include "nvl/ui/Window.h"

namespace nvl::test {

class TensorWindow final : public Window {
public:
    explicit TensorWindow(const std::string &title, Pos<2> shape);
    void predraw() override;

    void line_box(const Color &color, const Box<2> &box) override;
    void fill_box(const Color &color, const Box<2> &box) override;

    void line_cube(const Color &, const Box<3> &) override {}
    void fill_cube(const Color &, const Box<3> &) override {}

    void line(const Color &, const Line<2> &) override {}
    void line(const Color &, const Line<3> &) override {}

    void text(const Color &color, const Pos<2> &pos, I64 font_size, std::string_view text) override;
    void centered_text(const Color &color, const Pos<2> &pos, I64 font_size, std::string_view text) override;

    void set_view_offset(const ViewOffset &) override {}
    void end_view_offset(const ViewOffset &) override {}
    pure bool should_close() const override { return false; }
    pure I64 height() const override { return tensor_.shape()[1]; }
    pure I64 width() const override { return tensor_.shape()[0]; }
    pure I64 fps() const override { return 0; }

    template <typename Event, typename... Args>
    void send_event(Args &&...args) {
        pending_events_.push_back(InputEvent::get<Event>(std::forward<Args>(args)...));
    }

    List<InputEvent> detect_events() override {
        List<InputEvent> result = pending_events_;
        pending_events_.clear();
        return result;
    }

    pure const Tensor<2, Color> &tensor() const { return tensor_; }

private:
    std::string title_;
    Tensor<2, Color> tensor_;
    List<InputEvent> pending_events_;
};

inline void print_10x10_tensor(const Tensor<2, Color> &tensor) {
    std::cout << " 0123456789" << std::endl;
    for (int64_t i = 0; i < 10; ++i) {
        std::cout << i << "";
        for (int64_t j = 0; j < 10; ++j) {
            auto elem = tensor[{j, i}]; // Note: transposed
            std::cout << (elem == Color::kBlack ? "X" : ".");
        }
        std::cout << std::endl;
    }
}

} // namespace nvl::test
