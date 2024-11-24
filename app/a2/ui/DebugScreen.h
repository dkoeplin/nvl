#pragma once

#include "a2/world/WorldA2.h"
#include "nvl/actor/Actor.h"
#include "nvl/ui/Screen.h"

namespace a2 {

class DebugScreen final : public AbstractScreen {
public:
    class_tag(DebugScreen, AbstractScreen);
    explicit DebugScreen(Window *window, WorldA2 *world);

protected:
    void draw() override;
    void tick() override;

    WorldA2 *world_;
};

} // namespace a2
