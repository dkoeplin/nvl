#pragma once

#include "a0/tool/Tool.h"
#include "nvl/entity/Block.h"
#include "nvl/geo/Tuple.h"

namespace nvl {

class BlockCreator final : public Tool<2> {
public:
    class_tag(BlockCreator, Tool<2>);
    explicit BlockCreator(AbstractScreen *parent, World<2> *world);

    void tick() override {}
    void draw() override;

private:
    std::unique_ptr<Block<2>> pending_ = nullptr;
    Pos<2> init_ = Pos<2>::zero;
};

} // namespace nvl
