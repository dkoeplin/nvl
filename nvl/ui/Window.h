#pragma once

#include <string_view>

#include "nvl/data/List.h"
#include "nvl/data/Set.h"
#include "nvl/geo/Box.h"
#include "nvl/geo/Pos.h"
#include "nvl/macros/Pure.h"
#include "nvl/ui/Key.h"
#include "nvl/ui/Mouse.h"
#include "nvl/ui/Screen.h"

namespace nvl {

struct Color;

class Window {
public:
    enum class MouseMode {
        kViewport, // Invisible mouse, always centered.
        kStandard, // Visible mouse, moved by user.
    };

    class Offset {
    public:
        explicit Offset(Window *parent, const Pos<2> &offset);
        ~Offset();

    private:
        Window *parent_;
        Maybe<Pos<2>> prev_offset_;
    };

    explicit Window(std::string_view title, Pos<2> shape);
    virtual ~Window() = default;

    virtual void draw() = 0;
    virtual void tick() = 0;

    template <typename T, typename... Args>
    T *open(Args &&...args) {
        children_.push_back(Screen::get<T>(this, std::forward<Args>(args)...));
        return children_.back().dyn_cast<T>();
    }

    /// Returns the current mouse position in window coordinates.
    pure Pos<2> mouse_coord() const { return curr_mouse_.has_value() ? *curr_mouse_ : center(); }

    /// Returns the delta between current and previous mouse position, in window coordinates.
    pure Pos<2> mouse_delta() const {
        return (curr_mouse_.has_value() && prev_mouse_.has_value()) ? *curr_mouse_ - *prev_mouse_ : Pos<2>::zero;
    }

    pure I64 scroll_x() const { return scroll_[0]; }
    pure I64 scroll_y() const { return scroll_[1]; }

    pure const Set<Key> &pressed_keys() const { return pressed_keys_; }
    pure const Set<Mouse> &pressed_mouse() const { return pressed_mouse_; }

    pure bool is_pressed(const Key key) const { return pressed_keys_.has(key); }
    pure bool is_pressed(const Mouse mouse) const { return pressed_mouse_.has(mouse); }

    virtual void line_rectangle(const Color &color, const Box<2> &box) = 0;
    virtual void fill_rectangle(const Color &color, const Box<2> &box) = 0;

    /// Draws the given text with the top-left of the text at `pos`.
    virtual void text(const Color &color, const Pos<2> &pos, I64 font_size, std::string_view text) = 0;

    /// Draws the given text with the center of the text at `pos`.
    virtual void centered_text(const Color &color, const Pos<2> &pos, I64 font_size, std::string_view text) = 0;

    virtual void set_view_offset(const Maybe<Pos<2>> &offset) = 0;

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

    /// Returns the window view range in window coordinates.
    pure Box<2> bbox() const { return {{0, 0}, {width(), height()}}; }

    /// Returns the center of the window in window coordinates.
    pure Pos<2> center() const { return Pos<2>(width(), height()) / 2; }

protected:
    friend class Offset;

    Maybe<Pos<2>> offset_ = None;

    Set<Key> pressed_keys_;
    Set<Mouse> pressed_mouse_;
    Maybe<Pos<2>> curr_mouse_ = None;
    Maybe<Pos<2>> prev_mouse_ = None;
    Pos<2> scroll_ = Pos<2>::zero;

    List<Screen> children_;

    MouseMode mouse_mode_ = MouseMode::kStandard;
};

} // namespace nvl
