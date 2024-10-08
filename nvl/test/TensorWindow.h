#pragma once

#include "nvl/data/Tensor.h"
#include "nvl/draw/Color.h"
#include "nvl/ui/Window.h"

namespace nvl::test {

class TensorWindow final : public Window {
public:
    explicit TensorWindow(std::string_view title, Pos<2> shape);
    void draw() override;
    void tick() override;
    void line_rectangle(const Color &color, const Box<2> &box) override;
    void fill_rectangle(const Color &color, const Box<2> &box) override;
    void text(const Color &color, const Pos<2> &pos, I64 font_size, std::string_view text) override;
    void centered_text(const Color &color, const Pos<2> &pos, I64 font_size, std::string_view text) override;

    void set_view_offset(const Maybe<Pos<2>> &offset) override;
    pure bool should_close() const override { return false; }
    pure I64 height() const override { return tensor_.shape()[1]; }
    pure I64 width() const override { return tensor_.shape()[0]; }
    pure I64 fps() const override { return 0; }

    template <typename Event, typename... Args>
    void send_event(Args &&...args) {
        events_.push_back(InputEvent::get<Event>(std::forward<Args>(args)...));
    }

    pure const Tensor<2, Color> &tensor() const { return tensor_; }

private:
    std::string title_;
    Tensor<2, Color> tensor_;
    List<InputEvent> events_;
};

} // namespace nvl::test
