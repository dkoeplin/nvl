#pragma once

#include "a0/tool/Tool.h"

namespace nvl {

class BlockBreaker final : public Tool<2> {
public:
    class_tag(BlockBreaker, Tool<2>);
    explicit BlockBreaker(AbstractScreen *parent, World<2> *world);

    void tick() override {}
    void draw() override;

private:
    I64 radius_ = 20;
};

} // namespace nvl
