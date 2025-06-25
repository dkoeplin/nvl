#pragma once

#include "a0/tool/Tool.h"
#include "nvl/actor/Actor.h"

namespace nvl {

class BlockRemover final : public Tool<2> {
public:
    class_tag(BlockRemover, Tool<2>);
    explicit BlockRemover(AbstractScreen *parent, World<2> *world);

    void tick() override {}
    void draw() override;

private:
    Actor hovered_ = nullptr;
};

} // namespace nvl