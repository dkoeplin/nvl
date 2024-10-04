#pragma once

#include "nvl/geo/Pos.h"
#include "nvl/macros/Abstract.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/Pure.h"
#include "nvl/reflect/Castable.h"
#include "nvl/ui/UserInterface.h"

namespace nvl {

template <U64 N>
struct Tool;

template <U64 N>
class World;

template <U64 N>
abstract class AbstractTool : public UserInterface,
                              public Castable<Tool<N>, AbstractTool<N>, std::shared_ptr<Tool<N>>>::BaseClass {
public:
    class_tag(AbstractTool);
    explicit AbstractTool(World<N> *world) : world_(world) {}

protected:
    World<N> *world_;
};

template <U64 N>
struct Tool final : Castable<Tool<N>, AbstractTool<N>, std::shared_ptr<Tool<N>>> {};

} // namespace nvl
