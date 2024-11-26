#pragma once

#include "nvl/ui/GlowEffect.h"
#include "nvl/ui/Screen.h"

namespace a2 {

using namespace nvl;
struct WorldA2;

class PauseScreen final : public AbstractScreen {
public:
    class_tag(PauseScreen, AbstractScreen);
    explicit PauseScreen(AbstractScreen *parent, WorldA2 *world);

protected:
    void draw() override;
    void tick() override;

    GlowEffect glow = GlowEffect(3, 512, 756);
    WorldA2 *world_;
};

} // namespace a2
