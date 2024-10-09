#include "nvl/ui/RayWindow.h"

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

RayWindow::RayWindow(std::string_view title, Pos<2> shape) : Window(title, shape) {
    InitWindow(shape[0], shape[1], title.data());
    SetWindowFocused();
    MaximizeWindow();
    ToggleFullscreen();
}

RayWindow::~RayWindow() { CloseWindow(); }

void RayWindow::draw() {
    BeginDrawing();
    ClearBackground(raycolor(RAYWHITE));
    for (auto &child : children_) {
        child->draw_all();
    }
    // If SetTargetFPS is used, this sleeps
    EndDrawing();
}

void RayWindow::tick() {
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

    for (auto &child : children_) {
        events = child->tick_all(events);
    }
}

void RayWindow::line_rectangle(const Color &color, const Box<2> &box) {
    const Pos<2> shape = box.shape();
    DrawRectangleLines(box.min[0], box.min[1], shape[0], shape[1], raycolor(color));
}

void RayWindow::fill_rectangle(const Color &color, const Box<2> &box) {
    const Pos<2> shape = box.shape();
    DrawRectangle(box.min[0], box.min[1], shape[0], shape[1], raycolor(color));
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

void RayWindow::set_view_offset(const Maybe<Pos<2>> &offset) {
    if (offset_.has_value()) {
        EndMode2D();
    }
    offset_ = offset;
    if (offset_.has_value()) {
        Camera2D camera;
        camera.offset = Vector2{0, 0};
        camera.rotation = 0.0f;
        camera.zoom = 1.0f;
        camera.target = Vector2{static_cast<float>((*offset_)[0]), static_cast<float>((*offset_)[1])};
        BeginMode2D(camera);
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
