#pragma once

#include "nvl/geo/Pos.h"
#include "nvl/macros/Abstract.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/Pure.h"
#include "nvl/reflect/Castable.h"

namespace nvl {

template <U64 N>
struct Tool;

template <U64 N>
class World;

template <U64 N>
abstract class AbstractTool : public Castable<Tool<N>, AbstractTool<N>, std::shared_ptr<Tool<N>>>::BaseClass {
public:
    class_tag(AbstractTool);
    AbstractTool(World<N> *world) : world_(world) {}

    virtual void draw(Window &window) = 0;
    virtual void exit(Pos<2> pt) = 0;
    virtual void down(Pos<2> pt) = 0;
    virtual void move(Pos<2> pt) = 0;
    virtual void drag(Pos<2> pt) = 0;
    virtual void up(Pos<2> pt) = 0;

    void tick() { cooldown_ = std::max(cooldown_ - 1, 0LL); }

protected:
    pure I64 kMaxCooldown() const { return 0; }
    I64 cooldown_ = 0;
    World<N> *world_;
};

template <U64 N>
struct Tool final : Castable<Tool<N>, AbstractTool<N>, std::shared_ptr<Tool<N>>> {};

} // namespace nvl
