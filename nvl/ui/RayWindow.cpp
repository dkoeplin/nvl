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

void RayWindow::draw() {
    BeginDrawing();
    ClearBackground(raycolor(background_));
    for (auto &child : children_) {
        child->draw_all();
    }
    // If SetTargetFPS is used, this sleeps
    EndDrawing();
}

void RayWindow::feed() {
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
    scroll_ = {static_cast<I64>(scroll_x), static_cast<I64>(scroll_y)};
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

    for (auto iter = children_.begin(); iter != children_.end() && !events.empty(); ++iter) {
        events = (*iter)->feed_all(events);
    }
}

void RayWindow::line_box(const Color &color, const Box<2> &box) {
    const Pos<2> shape = box.shape();
    DrawRectangleLines(box.min[0], box.min[1], shape[0], shape[1], raycolor(color));
}

void RayWindow::fill_box(const Color &color, const Box<2> &box) {
    const Pos<2> shape = box.shape();
    DrawRectangle(box.min[0], box.min[1], shape[0], shape[1], raycolor(color));
}

void RayWindow::line_cube(const Color &color, const Box<3> &cube) {
    const Pos<3> shape = cube.shape();
    const Pos<3> center = cube.min + shape / 2;
    // Cube position is the _center_ position for raylib
    Vector3 pos;
    pos.x = center[0];
    pos.y = center[1];
    pos.z = center[2];
    DrawCubeWires(pos, shape[0], shape[1], shape[2], raycolor(color));
}

void RayWindow::fill_cube(const Color &color, const Box<3> &cube) {
    const Pos<3> shape = cube.shape();
    const Pos<3> center = cube.min + shape / 2;
    // Cube position is the _center_ position for raylib
    Vector3 pos;
    pos.x = center[0];
    pos.y = center[1];
    pos.z = center[2];
    DrawCube(pos, shape[0], shape[1], shape[2], raycolor(color));
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

void RayWindow::set_view_offset(const ViewOffset &offset) {
    if (auto *view2d = offset.dyn_cast<View2D>()) {
        Camera2D camera;
        camera.offset = Vector2{0, 0};
        camera.rotation = 0.0f;
        camera.zoom = 1.0f;
        camera.target = Vector2{static_cast<float>(view2d->offset[0]), static_cast<float>(view2d->offset[1])};
        BeginMode2D(camera);
    } else if (auto *view3d = offset.dyn_cast<View3D>()) {
        Camera3D camera;
        camera.projection = CAMERA_PERSPECTIVE;
        camera.fovy = view3d->fov;
        camera.up = {0.0f, -1.0f, 0.0f};
        camera.position = Vector3{.x = static_cast<float>(view3d->offset[0]),
                                  .y = static_cast<float>(view3d->offset[1]),
                                  .z = static_cast<float>(view3d->offset[2])};
        const auto target = view3d->project();
        camera.target = {
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

} // namespace nvl
