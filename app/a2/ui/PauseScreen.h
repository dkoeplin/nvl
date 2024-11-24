#pragma once

#include "nvl/ui/GlowEffect.h"
#include "nvl/ui/Screen.h"

namespace a2 {

using namespace nvl;

class PauseScreen final : public AbstractScreen {
public:
    class_tag(PauseScreen, AbstractScreen);
    explicit PauseScreen(Window *window);

protected:
    void draw() override;
    void tick() override;

    GlowEffect glow = GlowEffect(3, 512, 756);
};

} // namespace a2
