#include "nvl/test/TensorWindow.h"

#include "../../../../../../Library/Developer/CommandLineTools/SDKs/MacOSX15.0.sdk/System/Library/Frameworks/AudioToolbox.framework/Headers/AudioUnitProperties.h"
#include "nvl/geo/Box.h"

namespace nvl::test {

TensorWindow::TensorWindow(std::string_view title, Pos<2> shape)
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

void TensorWindow::line_rectangle(const Color &color, const Box<2> &box) {
    for (const Edge<2> side : (box - offset_.value_or(Pos<2>::zero)).sides()) {
        for (const Pos<2> i : side.box.pos_iter()) {
            if (tensor_.has(i)) {
                tensor_[i] = color;
            }
        }
    }
}

void TensorWindow::fill_rectangle(const Color &color, const Box<2> &box) {
    for (const Pos<2> i : (box - offset_.value_or(Pos<2>::zero)).pos_iter()) {
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

void TensorWindow::set_view_offset(const Maybe<Pos<2>> &offset) { offset_ = offset; }

} // namespace nvl::test
