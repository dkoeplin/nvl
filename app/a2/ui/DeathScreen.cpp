#include "a2/ui/DeathScreen.h"

#include "a2/action/Teleport.h"
#include "a2/entity/Player.h"
#include "a2/world/WorldA2.h"
#include "nvl/actor/Actor.h"
#include "nvl/geo/Box.h"
#include "nvl/ui/Window.h"

namespace a2 {

DeathScreen::DeathScreen(AbstractScreen *parent, WorldA2 *world) : AbstractScreen(parent), world_(world) {
    on_key_down[Key::Any] = [this] {
        if (ticks > Player::kRespawnTicks) {
            auto &view = world_->view3d();
            auto *player = world_->player;
            world_->send<Teleport>(nullptr, Actor(player), player->spawn);
            view.angle = 90;
            view.pitch = 10;
            world_->paused = false;
            close();
        }
    };
}

void DeathScreen::draw() {
    const auto center = window_->center();
    auto pos = center;
    pos[1] += 80;

    const Color color = Color::kRed.highlight(glow);
    window_->fill_box(color, Box({0, 0}, window_->shape()));
    window_->centered_text(Color::kRed, center, 50, "OH NO YOU DIED");
    if (ticks >= Player::kRespawnTicks) {
        window_->centered_text(Color::kBlack, pos, 30, "Press Any Key to Respawn");
    }
}

void DeathScreen::tick() {
    glow.advance();
    ticks += 1;
}

} // namespace a2
