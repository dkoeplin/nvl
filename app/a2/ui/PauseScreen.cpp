#include "a2/ui/PauseScreen.h"

#include "a2/world/WorldA2.h"
#include "nvl/ui/Window.h"

namespace a2 {

PauseScreen::PauseScreen(AbstractScreen *parent, WorldA2 *world) : AbstractScreen(parent), world_(world) {
    on_key_down[Key::Any] = [this] {
        world_->paused = false;
        close();
    };
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

void PauseScreen::tick() {
    std::cout << "PAUSE SCREEN TICK" << std::endl;
    glow.advance();
}

} // namespace a2
