#pragma once

#include "nvl/macros/Abstract.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/Pure.h"
#include "nvl/reflect/Castable.h"
#include "nvl/ui/Screen.h"

namespace nvl {

template <U64 N>
class World;

template <U64 N>
abstract class Tool : public AbstractScreen {
public:
    class_tag(Tool, AbstractScreen);
    explicit Tool(Window *window, World<N> *world) : AbstractScreen(window), world_(world) {}

protected:
    World<N> *world_;
};

} // namespace nvl
