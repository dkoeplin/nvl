#include "nvl/test/TestWindow.h"

#include "nvl/geo/Box.h"

namespace nvl::test {

void TestWindow::line_rectangle(const Color &color, const Box<2> &box) {
    for (const Edge<2> side : (box - offset_.value_or(Pos<2>::zero)).sides()) {
        for (const Pos<2> i : side.box.pos_iter()) {
            if (tensor_.has(i)) {
                tensor_[i] = color;
            }
        }
    }
}

void TestWindow::fill_rectangle(const Color &color, const Box<2> &box) {
    for (const Pos<2> i : (box - offset_.value_or(Pos<2>::zero)).pos_iter()) {
        if (tensor_.has(i)) {
            tensor_[i] = color;
        }
    }
}

} // namespace nvl::test
