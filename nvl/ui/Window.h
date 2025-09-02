#pragma once

#include <string_view>

#include "Color.h"
#include "nvl/data/List.h"
#include "nvl/data/Set.h"
#include "nvl/geo/Line.h"
#include "nvl/geo/Tuple.h"
#include "nvl/geo/Volume.h"
#include "nvl/macros/Pure.h"
#include "nvl/time/Duration.h"
#include "nvl/ui/Key.h"
#include "nvl/ui/Mouse.h"
#include "nvl/ui/Screen.h"
#include "nvl/ui/ViewOffset.h"

namespace nvl {

struct Color;

class Window : public AbstractScreen {
public:
    class_tag(Window, AbstractScreen);

    enum class MouseMode {
        kViewport, // Invisible mouse, always centered.
        kStandard, // Visible mouse, moved by user.
    };

    explicit Window(const std::string &title, Pos<2> shape);

    void draw() final;
    void tick() final;
    void react() final;

    /// Returns the current mouse position in window coordinates.
    pure Pos<2> mouse_coord() const { return curr_mouse_.has_value() ? *curr_mouse_ : Pos<2>::zero; }

    /// Returns the delta between current and previous mouse position, in window coordinates.
    pure Pos<2> mouse_delta() const {
        return curr_mouse_.has_value() && prev_mouse_.has_value() ? *curr_mouse_ - *prev_mouse_ : Pos<2>::zero;
    }

    pure F64 scroll_x() const { return scroll_[0]; }
    pure F64 scroll_y() const { return scroll_[1]; }

    pure const Set<Key> &pressed_keys() const { return pressed_keys_; }
    pure const Set<Mouse> &pressed_mouse() const { return pressed_mouse_; }

    pure bool pressed(const Set<Key> &keys) const {
        return keys.values().exists([&](const Key &key) { return pressed_keys_.has(key); });
    }
    pure bool pressed(const Key key) const { return pressed_keys_.has(key); }
    pure bool down(const Mouse mouse) const { return pressed_mouse_.has(mouse); }

    ////// 2D Drawing

    /// Draws a box in the given coordinates.
    virtual void line_box(const Color &color, const Box<2> &box) = 0;
    virtual void fill_box(const Color &color, const Box<2> &box) = 0;

    /// Draws a line at the given coordinates
    virtual void line(const Color &color, const Line<2> &line) = 0;

    /// Draws the given text with the top-left of the text at `pos`.
    virtual void text(const Color &color, const Pos<2> &pos, I64 font_size, std::string_view text) = 0;

    /// Draws the given text with the center of the text at `pos`.
    virtual void centered_text(const Color &color, const Pos<2> &pos, I64 font_size, std::string_view text) = 0;

    ////// 3D Drawing

    /// Draws a cube at the given coordinates.
    virtual void line_cube(const Color &color, const Box<3> &cube) = 0;
    virtual void fill_cube(const Color &color, const Box<3> &cube) = 0;

    virtual void line(const Color &color, const Line<3> &line) = 0;

    void push_view(const ViewOffset &offset);
    void pop_view();

    virtual void set_mouse_mode(const MouseMode mode) {
        mouse_mode_ = mode;
        prev_mouse_ = None;
        curr_mouse_ = None;
    }

    pure virtual bool should_close() const = 0;

    /// Returns the window height in window coordinates.
    pure virtual I64 height() const = 0;

    /// Returns the window width in window coordinates.
    pure virtual I64 width() const = 0;

    pure virtual I64 fps() const = 0;

    /// Returns the shape of this window in window coordinates (width x height).
    pure Pos<2> shape() const { return {width(), height()}; }

    /// Returns the window view range in window coordinates.
    pure Box<2> bbox() const { return {Pos<2>::zero, shape()}; }

    /// Returns the center of the window in window coordinates.
    pure Pos<2> center() const { return shape() / 2; }

    void set_background(const Color &color) { background_ = color; }

    /// Sets the target FPS for windows which draw to the screen.
    virtual void set_target_fps(U64) const {}

    const Duration &last_draw_time() const { return last_draw_time_; }
    const Duration &last_tick_time() const { return last_tick_time_; }

    /// Runs the outer game loop (tick, react, draw) on this window and all children.
    void loop(const Duration &nanos_per_tick, const Duration &nanos_per_draw);

protected:
    virtual List<InputEvent> detect_events() = 0;
    virtual void set_view_offset(const ViewOffset &offset) = 0;
    virtual void end_view_offset(const ViewOffset &offset) = 0;

    /// Called before/after drawing all child screens.
    virtual void predraw() {}
    virtual void postdraw() {}

    Set<Key> pressed_keys_;
    Set<Mouse> pressed_mouse_;
    Maybe<Tuple<2, I64>> curr_mouse_ = None;
    Maybe<Tuple<2, I64>> prev_mouse_ = None;
    Vec<2> scroll_ = Vec<2>::zero;

    List<ViewOffset> views_;

    MouseMode mouse_mode_ = MouseMode::kStandard;
    Color background_ = Color::kRayWhite;

    Duration last_draw_time_;
    Duration last_tick_time_;
};

} // namespace nvl
