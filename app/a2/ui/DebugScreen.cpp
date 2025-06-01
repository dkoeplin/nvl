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
    List<std::string> messages{
        "Draw:  " + window_->last_draw_time().to_string() + " (" + std::to_string(window_->fps()) + " FPS)",
        "Tick:  " + window_->last_tick_time().to_string(),
        "Loc:   " + offset.to_string(),
        "Look: " + look.to_string(),
        "V:    " + player->velocity().to_string(),
        "A:    " + player->accel().to_string(),
        "Target: " + target,
        "Angle:  " + std::to_string(view3d.angle),
        "Pitch:  " + std::to_string(view3d.pitch),
        "Alive: " + std::to_string(world_->num_awake()) + "/" + std::to_string(world_->num_alive())
    };
    // clang-format on

    I64 y = 10;
    for (const std::string &message : messages) {
        window_->text(Color::kBlack, {10, y}, 20, message);
        y += 30;
    }
}

void DebugScreen::tick() {}

} // namespace a2
