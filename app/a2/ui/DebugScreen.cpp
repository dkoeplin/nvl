#include "a2/ui/DebugScreen.h"

#include "a2/entity/Player.h"
#include "a2/world/WorldA2.h"
#include "nvl/actor/Actor.h"
#include "nvl/entity/Block.h"
#include "nvl/geo/Line.h"
#include "nvl/geo/Tuple.h"

namespace a2 {

DebugScreen::DebugScreen(AbstractScreen *parent, WorldA2 *world) : AbstractScreen(parent), world_(world) {
    on_key_down[Key::L] = [this] {
        for (const Actor &actor : world_->entities()) {
            if (const auto *block = actor.dyn_cast<Block<3>>()) {
                std::cout << block->bbox() << std::endl;
            }
        }
        const auto &view3d = world_->view3d();
        const Pos<3> offset = view3d.offset;
        const Vec<3> target = view3d.project();
        std::cout << "  Offset: " << offset << std::endl;
        std::cout << "  Target: " << target << std::endl;
    };
}

void DebugScreen::draw() {
    const auto &view3d = world_->view3d();
    const Vec<3> offset = real(view3d.offset);
    const Vec<3> target = view3d.project();
    const Line<3> line{offset, target};
    const Actor player(world_->player);
    std::string at = "N/A";
    if (const auto itx = world_->first_except(line, player)) {
        at = itx->actor.dyn_cast<Entity<3>>()->bbox().to_string();
    }

    window_->text(Color::kBlack, {10, 10}, 20, "FPS:  " + std::to_string(window_->fps()));
    window_->text(Color::kBlack, {10, 40}, 20, "Offset: " + offset.to_string());
    window_->text(Color::kBlack, {10, 70}, 20, "Target: " + target.to_string());
    window_->text(Color::kBlack, {10, 100}, 20, "At:    " + at);
    window_->text(Color::kBlack, {10, 130}, 20, "Angle: " + std::to_string(view3d.angle));
    window_->text(Color::kBlack, {10, 160}, 20, "Pitch: " + std::to_string(view3d.pitch));
    window_->text(Color::kBlack, {10, 190}, 20,
                  "Alive: " + std::to_string(world_->num_awake()) + "/" + std::to_string(world_->num_alive()));
}

void DebugScreen::tick() {}

} // namespace a2
