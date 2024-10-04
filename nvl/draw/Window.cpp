#include "nvl/draw/Window.h"

#include "nvl/draw/Color.h"
#include "nvl/geo/Box.h"
#include "raylib.h"

namespace nvl {

namespace {

void set_view_offset(const Pos<2> &offset) {
    Camera2D camera;
    camera.offset = Vector2{static_cast<float>(offset[0]), static_cast<float>(offset[1])};
    camera.rotation = 0;
    camera.zoom = 0;
    camera.target = Vector2{0, 0};
    BeginMode2D(camera);
}

} // namespace

Window::Offset::Offset(Window &parent, const Pos<2> &offset) : parent_(parent), prev_offset_(parent.offset_) {
    parent.offset_ = offset;
    if (parent.offset_.has_value()) {
        EndMode2D();
    }
    set_view_offset(offset);
}

Window::Offset::~Offset() {
    parent_.offset_ = prev_offset_;
    EndMode2D();
    if (prev_offset_.has_value()) {
        set_view_offset(*prev_offset_);
    }
}

Window::Window(const std::string_view title, const Pos<2> shape) {
    InitWindow(shape[0], shape[1], title.data());
    ToggleFullscreen();
}

Window::~Window() { CloseWindow(); }

void Window::draw() {
    ClearBackground(RAYWHITE);
    DrawText("TEXT!", 190, 200, 20, BLACK);
    DrawFPS(10, 10);
}

void Window::line_rectangle(const Color &color, const Box<2> &box) {
    const Pos<2> shape = box.shape();
    DrawRectangleLines(box.min[0], box.min[1], shape[0], shape[1], color.color32);
}

void Window::fill_rectangle(const Color &color, const Box<2> &box) {
    const Pos<2> shape = box.shape();
    DrawRectangle(box.min[0], box.min[1], shape[0], shape[1], color.color32);
}

bool Window::should_close() const { return WindowShouldClose(); }

} // namespace nvl
