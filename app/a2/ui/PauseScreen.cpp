#include "a2/ui/PauseScreen.h"

#include "nvl/ui/Window.h"

namespace a2 {

PauseScreen::PauseScreen(Window *window) : AbstractScreen(window) {
    on_key_down[Key::Any] = [this] { close(); };
}

void PauseScreen::draw() {
    const auto center = window_->center();
    auto pos = center;
    pos[1] += 80;

    const Color color = Color::kBlack.highlight(glow);
    window_->fill_box(color, Box({0, 0}, window_->shape()));
    window_->centered_text(Color::kBlack, window_->center(), 50, "PAUSED");
    window_->centered_text(Color::kBlack, pos, 30, "Press Any Key to Resume");
}

void PauseScreen::tick() { glow.advance(); }

} // namespace a2
