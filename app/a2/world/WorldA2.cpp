#include "a2/world/WorldA2.h"

#include "a2/entity/Player.h"
#include "a2/macros/Literals.h"
#include "a2/ui/DeathScreen.h"
#include "a2/ui/DebugScreen.h"
#include "a2/ui/PauseScreen.h"
#include "nvl/entity/Block.h"
#include "nvl/material/Bulwark.h"
#include "nvl/material/Material.h"
#include "nvl/material/TestMaterial.h"

namespace a2 {

namespace {
const std::vector kColors = {
    Color::kLightGray,  Color::kGray,    Color::kDarkGray, Color::kYellow,    Color::kGold,   Color::kOrange,
    Color::kPink,       Color::kRed,     Color::kMaroon,   Color::kMagenta,   Color::kGreen,  Color::kLime,
    Color::kDarkGreen,  Color::kSkyBlue, Color::kBlue,     Color::kDarkBlue,  Color::kPurple, Color::kViolet,
    Color::kDarkPurple, Color::kBeige,   Color::kBrown,    Color::kDarkBrown,
};
} // namespace

WorldA2::WorldA2(AbstractScreen *parent)
    : World<3>(parent, {.gravity_accel = 10_mps2,
                        .maximum_y = 10_m,
                        .ms_per_tick = a2::kMillisPerTick,
                        .pixels_per_meter = a2::kPixelsPerMeter,
                        .terminal_velocity = 53_mps}) {

    window_->set_background(Color::kSkyBlue);
    open<PauseScreen>(this);

    for (const auto &color : kColors) {
        materials.push_back(Material::get<TestMaterial>(color));
    }

    std::cout << "V: " << 30_mps << std::endl;
    std::cout << "G: " << kGravityAccel << std::endl;

    const Material bulwark = Material::get<Bulwark>(Color::kDarkGreen);
    spawn<Block<3>>(Pos<3>{-50_m, 0, -50_m}, Pos<3>{100_m, 1_m, 100_m}, bulwark);

    constexpr Pos<3> start{0, -2_m, 0};
    player = spawn<Player>(start);
    view3d().offset = start;
    view3d().angle = -90;
    view3d().pitch = 10;
    view3d().dist = 50_m;
    view3d().scale = static_cast<double>(kPixelsPerMeter);

    const std::vector colors{Color::kRed, Color::kGreen, Color::kBlue};
    I64 x = -250_cm;
    constexpr Pos<3> shape{1_m, 1_m, 1_m};
    for (const auto &color : colors) {
        const Pos<3> loc{x, -2_m, -2_m};
        spawn<Block<3>>(loc, shape, Material::get<TestMaterial>(color));
        x += 2_m;
    }
    const std::vector colors2{Color::kYellow, Color::kOrange, Color::kPurple};
    x = -250_cm;
    for (const auto &color : colors2) {
        const Pos<3> loc{x, -4_m, -2_m};
        spawn<Block<3>>(loc, shape, Material::get<TestMaterial>(color));
        x += 2_m;
    }

    on_key_down[Key::P] = [this] { open<PauseScreen>(this); };
    on_key_down[Key::N] = [this] { spawn_random_cube(); };
}

void WorldA2::remove(const Actor &actor) {
    if (actor.isa<Player>()) {
        window_->open<DeathScreen>(this);
    } else {
        World::remove(actor);
    }
}

void WorldA2::spawn_random_cube() {
    const auto left = random.uniform<I64, I64>(-50_m, 50_m);
    const auto back = random.uniform<I64, I64>(-50_m, 50_m);
    const auto width = random.uniform<I64, I64>(10_cm, 5_m);
    const auto height = random.uniform<I64, I64>(10_cm, 5_m);
    const auto depth = random.uniform<I64, I64>(10_cm, 5_m);
    const auto top = std::min<I64>(0, entities_.bbox().min[1]) - height - 2;
    const auto color_idx = random.uniform<U64, U64>(0, materials.size() - 1);
    const auto material = materials.at(color_idx);
    const Pos<3> pos{left, top, back};
    const Pos<3> shape{width, height, depth};
    spawn<Block<3>>(pos, shape, material);
    prev_generated = ticks();
}

void WorldA2::tick() {
    return_if(paused);

    const auto prev_player_loc = player->loc();
    World::tick();

    const auto diff = player->loc() - prev_player_loc;
    view3d().offset += diff;

#if 1
    if (ticks() - prev_generated >= ticks_per_gen) {
        spawn_random_cube();
    }
#endif
}

void WorldA2::draw() { World::draw(); }

} // namespace a2