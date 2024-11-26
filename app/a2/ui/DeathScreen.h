#pragma once

#include "nvl/ui/GlowEffect.h"
#include "nvl/ui/Screen.h"

namespace a2 {

using namespace nvl;

struct WorldA2;

class DeathScreen final : public AbstractScreen {
public:
    class_tag(DeathScreen, AbstractScreen);
    explicit DeathScreen(AbstractScreen *parent, WorldA2 *world);

protected:
    void draw() override;
    void tick() override;

    GlowEffect glow = GlowEffect(10, 512, 900);
    U64 ticks = 0;
    WorldA2 *world_;
};

} // namespace a2
