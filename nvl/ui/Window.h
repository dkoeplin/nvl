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

template <U64 N>
class Box;

class Window {
public:
    enum class MouseMode {
        kViewport, // Invisible mouse, always centered.
        kStandard, // Visible mouse, moved by user.
    };

    class Offset {
    public:
        explicit Offset(Window &parent, const Pos<2> &offset);
        ~Offset();

    private:
        Window &parent_;
        Maybe<Pos<2>> prev_offset_;
    };

    explicit Window(std::string_view title, Pos<2> shape);
    ~Window();

    void draw();
    void tick();

    template <typename T, typename... Args>
    void open(Args &&...args) {
        children_.push_back(Screen::get<T>(this, std::forward<Args>(args)...));
    }

    /// Returns the current mouse position in window coordinates.
    pure Pos<2> mouse_coord() const { return curr_mouse_; }

    pure const Set<Key> &pressed_keys() const { return pressed_keys_; }
    pure const Set<Mouse> &pressed_mouse() const { return pressed_mouse_; }

    pure bool is_pressed(const Key key) const { return pressed_keys_.has(key); }
    pure bool is_pressed(const Mouse mouse) const { return pressed_mouse_.has(mouse); }

    void line_rectangle(const Color &color, const Box<2> &box);
    void fill_rectangle(const Color &color, const Box<2> &box);

    /// Draws the given text with the top-left of the text at `pos`.
    void text(const Color &color, const Pos<2> &pos, U64 font_size, std::string_view text);

    /// Draws the given text with the center of the text at `pos`.
    void centered_text(const Color &color, const Pos<2> &pos, U64 font_size, std::string_view text);

    pure bool should_close() const;

    /// Returns the window height in window coordinates.
    pure I64 height() const;

    /// Returns the window width in window coordinates.
    pure I64 width() const;

    /// Returns the window view range in window coordinates.
    pure Box<2> bbox() const { return {{0, 0}, {width(), height()}}; }

    /// Returns the center of the window in window coordinates.
    pure Pos<2> center() const { return Pos<2>(width(), height()) / 2; }

private:
    friend class Offset;

    Maybe<Pos<2>> offset_ = None;

    Set<Key> pressed_keys_;
    Set<Mouse> pressed_mouse_;
    Pos<2> curr_mouse_ = Pos<2>::zero;
    Pos<2> prev_mouse_ = Pos<2>::zero;

    List<Screen> children_;

    MouseMode mouse_mode_ = MouseMode::kStandard;
};

} // namespace nvl
