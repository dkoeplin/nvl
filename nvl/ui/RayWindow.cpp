#include "nvl/ui/RayWindow.h"

#include "nvl/draw/Color.h"
#include "raylib.h"

namespace nvl {

namespace {

void set_view_offset(const Pos<2> &offset) {}

constexpr ::Color raycolor(const Color &color) {
    return ::Color{.r = static_cast<U8>(color.r),
                   .g = static_cast<U8>(color.g),
                   .b = static_cast<U8>(color.b),
                   .a = static_cast<U8>(color.a)};
}

} // namespace

RayWindow::RayWindow(std::string_view title, Pos<2> shape) : Window(title, shape) {
    InitWindow(shape[0], shape[1], title.data());
    ToggleFullscreen();
}

RayWindow::~RayWindow() { CloseWindow(); }

void RayWindow::draw() {
    BeginDrawing();
    ClearBackground(raycolor(RAYWHITE));
    for (auto &child : children_) {
        child->draw_all();
    }
    DrawFPS(10, 10);
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
    if (prev_mouse_ != curr_mouse_) {
        events.push_back(InputEvent::get<MouseMove>(pressed_mouse_));
    }
    for (auto iter = children_.begin(); !events.empty() && iter != children_.end(); ++iter) {
        events = (*iter)->tick_all(events);
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

void RayWindow::set_view_offset(const Maybe<Pos<2>> &offset) override {
    if (offset_.has_value()) {
        EndMode2D();
    }
    offset_ = offset;
    if (offset_.has_value()) {
        Camera2D camera;
        camera.offset = Vector2{static_cast<float>((*offset)[0]), static_cast<float>((*offset)[1])};
        camera.rotation = 0;
        camera.zoom = 0;
        camera.target = Vector2{0, 0};
        BeginMode2D(camera);
    }
}

bool RayWindow::should_close() const { return WindowShouldClose(); }
I64 RayWindow::height() const { return GetScreenHeight(); }
I64 RayWindow::width() const { return GetScreenWidth(); }

} // namespace nvl
