#include "nvl/ui/RayWindow.h"

#include "nvl/macros/Unreachable.h"
#include "nvl/ui/Color.h"
#include "raylib.h"

namespace nvl {

namespace {

constexpr ::Color raycolor(const Color &color) {
    return ::Color{.r = static_cast<U8>(color.r),
                   .g = static_cast<U8>(color.g),
                   .b = static_cast<U8>(color.b),
                   .a = static_cast<U8>(color.a)};
}

} // namespace

RayWindow::RayWindow(const std::string &title, Pos<2> shape) : Window(title, shape) {
    InitWindow(shape[0], shape[1], title.data());
    SetWindowFocused();
    MaximizeWindow();
    ToggleFullscreen();
}

RayWindow::~RayWindow() { CloseWindow(); }

void RayWindow::predraw() {
    BeginDrawing();
    ClearBackground(raycolor(background_));
}

void RayWindow::postdraw() {
    // NOTE: If SetTargetFPS is used, this sleeps
    EndDrawing();
}

List<InputEvent> RayWindow::detect_events() {
    List<InputEvent> events;
    while (auto key = GetKeyPressed()) {
        pressed_keys_.emplace(static_cast<Key::Value>(key));
        events.push_back(InputEvent::get<KeyDown>(key));
    }
    for (auto button : Mouse::kButtons) {
        if (IsMouseButtonPressed(button)) {
            pressed_mouse_.emplace(button);
            events.push_back(InputEvent::get<MouseDown>(button));
        }
        if (IsMouseButtonReleased(button)) {
            pressed_mouse_.remove(button);
            events.push_back(InputEvent::get<MouseUp>(button));
        }
    }

    const auto [scroll_x, scroll_y] = GetMouseWheelMoveV();
    scroll_ = {scroll_x, scroll_y};
    if (scroll_x != 0) {
        events.push_back(InputEvent::get<MouseScroll>(Scroll::kHorizontal));
    }
    if (scroll_y != 0) {
        events.push_back(InputEvent::get<MouseScroll>(Scroll::kVertical));
    }

    List<Key> released;
    for (auto key : pressed_keys_) {
        if (IsKeyReleased(key)) {
            released.push_back(key);
            events.push_back(InputEvent::get<KeyUp>(key));
        }
    }
    pressed_keys_.remove(released.range());

    prev_mouse_ = curr_mouse_;
    curr_mouse_ = {GetMouseX(), GetMouseY()};

    if (prev_mouse_ != curr_mouse_ && prev_mouse_.has_value() && curr_mouse_.has_value()) {
        events.push_back(InputEvent::get<MouseMove>(pressed_mouse_));
    }
    return events;
}

void RayWindow::line_box(const Color &color, const Box<2> &box) {
    const Tuple<2, I64> shape = box.shape();
    DrawRectangleLines(box.min[0], box.min[1], shape[0], shape[1], raycolor(color));
}

void RayWindow::fill_box(const Color &color, const Box<2> &box) {
    const Tuple<2, I64> shape = box.shape();
    DrawRectangle(box.min[0], box.min[1], shape[0], shape[1], raycolor(color));
}

void RayWindow::line_cube(const Color &color, const Box<3> &cube) {
    const Vec<3> min = real(cube.min) / scale_;
    const Vec<3> shape = real(cube.shape()) / scale_;
    // Cube position is the _center_ position for raylib
    Vector3 pos;
    pos.x = static_cast<float>(min[0] + shape[0] / 2);
    pos.y = static_cast<float>(min[1] + shape[1] / 2);
    pos.z = static_cast<float>(min[2] + shape[2] / 2);
    DrawCubeWires(pos, shape[0], shape[1], shape[2], raycolor(color));
}

void RayWindow::fill_cube(const Color &color, const Box<3> &cube) {
    const Vec<3> min = real(cube.min) / scale_;
    const Vec<3> shape = real(cube.shape()) / scale_;
    // Cube position is the _center_ position for raylib
    Vector3 pos;
    pos.x = static_cast<float>(min[0] + shape[0] / 2);
    pos.y = static_cast<float>(min[1] + shape[1] / 2);
    pos.z = static_cast<float>(min[2] + shape[2] / 2);
    DrawCube(pos, shape[0], shape[1], shape[2], raycolor(color));
}

void RayWindow::line(const Color &color, const Line<2> &line) {
    const Vec<2> &a = line.a();
    const Vec<2> &b = line.b();
    DrawLine(a[0], a[1], b[0], b[1], raycolor(color));
}

void RayWindow::line(const Color &color, const Line<3> &line) {
    const Vec<3> &a = line.a() / scale_;
    const Vec<3> &b = line.b() / scale_;
    const Vector3 start{.x = static_cast<float>(a[0]), .y = static_cast<float>(a[1]), .z = static_cast<float>(a[2])};
    const Vector3 end{.x = static_cast<float>(b[0]), .y = static_cast<float>(b[1]), .z = static_cast<float>(b[2])};
    DrawLine3D(start, end, raycolor(color));
}

void RayWindow::text(const Color &color, const Pos<2> &pos, I64 font_size, std::string_view text) {
    DrawText(text.data(), pos[0], pos[1], font_size, raycolor(color));
}

void RayWindow::centered_text(const Color &color, const Pos<2> &pos, I64 font_size, std::string_view text) {
    static constexpr I64 kMinFontSize = 10;
    font_size = std::max(font_size, kMinFontSize);
    const I64 width = MeasureText(text.data(), font_size);
    const I64 height = font_size;
    const I64 x = pos[0] - width / 2;
    const I64 y = pos[1] - height / 2;
    this->text(color, {x, y}, font_size, text);
}

I64 RayWindow::fps() const { return GetFPS(); }

void RayWindow::set_view_offset(const ViewOffset &view) {
    if (auto *view2d = view.dyn_cast<View2D>()) {
        Camera2D camera;
        camera.offset = Vector2{0, 0};
        camera.rotation = 0.0f;
        camera.zoom = 1.0f;
        camera.target = Vector2{static_cast<float>(view2d->offset[0]), static_cast<float>(view2d->offset[1])};
        BeginMode2D(camera);
    } else if (auto *view3d = view.dyn_cast<View3D>()) {
        scale_ = view3d->scale;
        const Vec<3> offset = real(view3d->offset) / scale_;
        const Vec<3> target = view3d->project() / scale_;

        Camera3D camera;
        camera.projection = CAMERA_PERSPECTIVE;
        camera.fovy = view3d->fov;
        camera.up = {0.0f, -1.0f, 0.0f};
        camera.position = Vector3{
            .x = static_cast<float>(offset[0]), .y = static_cast<float>(offset[1]), .z = static_cast<float>(offset[2])};
        camera.target = Vector3{
            .x = static_cast<float>(target[0]), .y = static_cast<float>(target[1]), .z = static_cast<float>(target[2])};
        BeginMode3D(camera);
    } else {
        UNREACHABLE;
    }
}

void RayWindow::end_view_offset(const ViewOffset &offset) {
    if (offset.isa<View2D>()) {
        EndMode2D();
    } else if (offset.isa<View3D>()) {
        EndMode3D();
        scale_ = 1.0;
    } else {
        UNREACHABLE;
    }
}

void RayWindow::set_mouse_mode(const MouseMode mode) {
    if (mode == mouse_mode_)
        return;
    Window::set_mouse_mode(mode);
    if (mode == MouseMode::kViewport) {
        DisableCursor();
    } else {
        EnableCursor();
        SetMousePosition(width() / 2, height() / 2);
    }
}

bool RayWindow::should_close() const { return WindowShouldClose(); }
I64 RayWindow::height() const { return GetRenderHeight(); }
I64 RayWindow::width() const { return GetRenderWidth(); }

void RayWindow::set_target_fps(const U64 fps) const { SetTargetFPS(fps); }

} // namespace nvl
