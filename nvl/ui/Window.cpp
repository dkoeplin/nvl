#include "nvl/ui/Window.h"

namespace nvl {

Window::Offset::Offset(Window *parent, const Pos<2> &offset) : parent_(parent), prev_offset_(parent->offset_) {
    parent_->set_view_offset(offset);
}

Window::Offset::~Offset() { parent_->set_view_offset(prev_offset_); }

Window::Window(const std::string &, Pos<2>) {}

} // namespace nvl
