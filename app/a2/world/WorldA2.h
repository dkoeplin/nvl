#pragma once

#include "nvl/entity/Entity.h"
#include "nvl/macros/Literals.h"
#include "nvl/reflect/ClassTag.h"
#include "nvl/world/World.h"

namespace a2 {

using namespace nvl;

struct Player;

struct WorldA2 final : World<3> {
    class_tag(WorldA2, World<3>);

    explicit WorldA2(AbstractScreen *parent);

    void remove(const Actor &actor) override;

    void spawn_random_cube();

    void tick() override;
    void draw() override;

    View3D &view3d() { return *view_.dyn_cast<View3D>(); }

    Player *player;
    U64 prev_generated = 0;
    U64 ticks_per_gen = 50;
    bool paused = false;

    std::vector<Material> materials;
};

} // namespace a2
