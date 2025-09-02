#include "nvl/test/TensorWindow.h"

#include "nvl/geo/Volume.h"

namespace nvl::test {

TensorWindow::TensorWindow(const std::string &title, const Pos<2> shape)
    : Window(title, shape), title_(title), tensor_(shape, Color::kWhite) {}

void TensorWindow::predraw() {
    for (Color &pixel : tensor_) {
        pixel = Color::kWhite;
    }
}

void TensorWindow::line_box(const Color &color, const Box<2> &box) {
    Pos<2> view = Pos<2>::zero;
    if (!views_.empty() && views_.back().isa<View2D>()) {
        view = views_.get_back()->dyn_cast<View2D>()->offset;
    }
    for (const auto &side : (box - view).faces()) {
        for (const Tuple<2, I64> i : side.box.indices()) {
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
    for (const Tuple<2, I64> i : (box - view).indices()) {
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
