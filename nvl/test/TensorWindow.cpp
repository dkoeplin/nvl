#include "nvl/test/TensorWindow.h"

#include "nvl/geo/Box.h"

namespace nvl::test {

TensorWindow::TensorWindow(const std::string &title, Pos<2> shape)
    : Window(title, shape), title_(title), tensor_(shape, Color::kWhite) {}

void TensorWindow::tick() {
    for (auto &child : children_) {
        child->tick_all();
    }
}

void TensorWindow::feed() {
    for (auto iter = children_.begin(); iter != children_.end() && !events_.empty(); ++iter) {
        events_ = (*iter)->feed_all(events_);
    }
    events_.clear();
}

void TensorWindow::draw() {
    for (Color &pixel : tensor_) {
        pixel = Color::kWhite;
    }
    for (auto &child : children_) {
        child->draw_all();
    }
}

void TensorWindow::line_box(const Color &color, const Box<2> &box) {
    Pos<2> view = Pos<2>::zero;
    if (!views_.empty() && views_.back().isa<View2D>()) {
        view = views_.get_back()->dyn_cast<View2D>()->offset;
    }
    for (const Edge<2> side : (box - view).faces()) {
        for (const Pos<2> i : side.box.pos_iter()) {
            if (tensor_.has(i)) {
                tensor_[i] = color;
            }
        }
    }
}

void TensorWindow::fill_box(const Color &color, const Box<2> &box) {
    Pos<2> view = Pos<2>::zero;
    if (!views_.empty() && views_.back().isa<View2D>()) {
        view = views_.get_back()->dyn_cast<View2D>()->offset;
    }
    for (const Pos<2> i : (box - view).pos_iter()) {
        if (tensor_.has(i)) {
            tensor_[i] = color;
        }
    }
}

void TensorWindow::text(const Color &, const Pos<2> &, const I64, std::string_view) {
    // Nothing yet
}

void TensorWindow::centered_text(const Color &, const Pos<2> &, const I64, std::string_view) {
    // Nothing yet
}

} // namespace nvl::test
