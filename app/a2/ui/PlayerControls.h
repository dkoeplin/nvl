#pragma once

#include "a2/world/WorldA2.h"
#include "nvl/tool/Tool.h"

namespace a2 {
using namespace nvl;

struct PlayerControls final : Tool<3> {
    explicit PlayerControls(AbstractScreen *parent, WorldA2 *world) : Tool(parent, world), world_(world) {}
    void tick() override;
    void draw() override;
    WorldA2 *world_; // NOTE: This shadows the world_ field in the parent class
};

} // namespace a2
