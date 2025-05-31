#include "a2/ui/DebugScreen.h"

#include "a2/entity/Player.h"
#include "a2/world/WorldA2.h"
#include "nvl/actor/Actor.h"
#include "nvl/entity/Block.h"
#include "nvl/geo/Line.h"
#include "nvl/geo/Tuple.h"

namespace a2 {

namespace {
std::string get_target(const WorldA2 *world) {
    const Actor player(world->player);
    const auto &view3d = world->view3d();
    const Line<3> line{real(view3d.offset), view3d.project()};

    if (const auto itx = world->first_except(line, player)) {
        return itx->actor.dyn_cast<Entity<3>>()->bbox().to_string();
    }
    return "N/A";
}
} // namespace

DebugScreen::DebugScreen(AbstractScreen *parent, WorldA2 *world) : AbstractScreen(parent), world_(world) {
    on_key_down[Key::L] = [this] {
        for (const Actor &actor : world_->entities()) {
            if (const auto *block = actor.dyn_cast<Block<3>>()) {
                std::cout << block->bbox() << std::endl;
            }
        }
        const auto &view3d = world_->view3d();
        const Pos<3> offset = view3d.offset;
        const Vec<3> look = view3d.project();
        const std::string target = get_target(world_);

        std::cout << "  Offset: " << offset << std::endl;
        std::cout << "  Look:   " << look << std::endl;
        std::cout << "  Target: " << target << std::endl;
    };
}

void DebugScreen::draw() {
    const auto &view3d = world_->view3d();
    const Vec<3> offset = real(view3d.offset) / view3d.scale;
    const Vec<3> look = view3d.project() / view3d.scale;
    const std::string target = get_target(world_);
    const Player *player(world_->player);

    // clang-format off
    window_->text(Color::kBlack, {10, 10}, 20,  "FPS:  " + std::to_string(window_->fps()));
    window_->text(Color::kBlack, {10, 40}, 20,  "Loc:  " + offset.to_string());
    window_->text(Color::kBlack, {10, 70}, 20,  "Look: " + look.to_string());
    window_->text(Color::kBlack, {10, 100}, 20, "V:    " + player->velocity().to_string());
    window_->text(Color::kBlack, {10, 130}, 20, "A:    " + player->accel().to_string());
    window_->text(Color::kBlack, {10, 160}, 20, "Target: " + target);
    window_->text(Color::kBlack, {10, 190}, 20, "Angle:  " + std::to_string(view3d.angle));
    window_->text(Color::kBlack, {10, 220}, 20, "Pitch:  " + std::to_string(view3d.pitch));
    window_->text(Color::kBlack, {10, 250}, 20,
                  "Alive: " + std::to_string(world_->num_awake()) + "/" + std::to_string(world_->num_alive()));
    // clang-format on
}

void DebugScreen::tick() {}

} // namespace a2
