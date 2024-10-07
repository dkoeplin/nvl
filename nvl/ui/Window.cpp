#include "nvl/ui/Window.h"

namespace nvl {

Window::Offset Window::Offset::Absolute(Window *parent, const Pos<2> &offset) { return Offset(parent, offset); }
Window::Offset Window::Offset::Relative(Window *parent, const Pos<2> &offset) {
    const Pos<2> pos = parent->offset_.has_value() ? *parent->offset_ - offset : offset;
    return Offset(parent, pos);
}

Window::Offset::Offset(Window *parent, const Pos<2> &offset) : parent_(parent), prev_offset_(parent->offset_) {
    parent_->set_view_offset(offset);
}

Window::Offset::~Offset() { parent_->set_view_offset(prev_offset_); }

Window::Window(std::string_view, Pos<2>) {}

} // namespace nvl
