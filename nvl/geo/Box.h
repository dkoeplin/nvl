#pragma once

#include <nvl/data/Range.h>

#include <functional>

#include "nvl/data/List.h"
#include "nvl/data/Maybe.h"
#include "nvl/enum/Dir.h"
#include "nvl/geo/Edge.h"
#include "nvl/geo/Pos.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/Pure.h"

namespace nvl {

template <U64 N>
class Box {
  public:
    static constexpr Box presorted(const Pos<N> &min, const Pos<N> &max) {
        Box box;
        box.min = min;
        box.max = max;
        return box;
    }

    static const Box kUnitBox;

    /// Returns a Box with `min` and `max` if min is strictly less than or equal to max.
    /// Returns None otherwise.
    /// Used to detect cases e.g. where intersection results in an empty Box.
    static Maybe<Box> get(const Pos<N> &min, const Pos<N> &max) {
        if (min.all_lte(max)) {
            return Box::presorted(min, max);
        }
        return None;
    }

    /// Returns a Box with only one point.
    static Box unit(const Pos<N> &pt) { return Box::presorted(pt, pt); }

    class pos_iterator {
      public:
        using value_type = Pos<N>;
        using pointer = Pos<N> *;
        using reference = Pos<N> &;
        using difference_type = std::ptrdiff_t;            // TODO: This likely isn't right
        using iterator_category = std::input_iterator_tag; // TODO: This may not be right

        static pos_iterator begin(const Box &box, const Pos<N> &step) { return pos_iterator(box, box.min, step); }
        static pos_iterator end(const Box &box, const Pos<N> &step) { return pos_iterator(box, None, step); }

        pos_iterator() : pos_iterator(kUnitBox, None, Pos<N>::fill(1)) {}

        const Pos<N> &operator*() const { return pos_.value(); }
        const Pos<N> *operator->() const { return &pos_.value(); }
        pos_iterator &operator++() {
            // Increment only does something if this is not the end iterator.
            if (pos_.has_value()) {
                auto &pos = pos_.value();
                I64 i = N - 1;
                pos[i] = pos[i] + step_[i];
                while (i >= 0 && pos[i] > box_.max[i]) {
                    pos[i] = box_.min[i];
                    i -= 1;
                    if (i >= 0) {
                        pos[i] += step_[i];
                    } else {
                        pos_ = None; // Reached end
                    }
                }
            }
            return *this;
        }
        pure bool operator==(const pos_iterator &rhs) const {
            return pos_ == rhs.pos_ && box_ == rhs.box_ && step_ == rhs.step_;
        }
        pure bool operator!=(const pos_iterator &rhs) const { return !(*this == rhs); }

      private:
        explicit pos_iterator(const Box &box, const Maybe<Pos<N>> &pos, const Pos<N> &step)
            : box_(box), pos_(pos), step_(step) {
            for (U64 i = 0; i < N; i++) {
                ASSERT(step_[i] != 0, "Invalid iterator step size of 0");
                ASSERT(step_[i] > 0, "TODO: Support negative step");
            }
        }

        Box box_;
        Maybe<Pos<N>> pos_;
        Pos<N> step_;
    };

    class box_iterator {
      public:
        using value_type = Box;
        using pointer = Box *;
        using reference = Box &;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::input_iterator_tag;

        static box_iterator begin(const Box &box, const Pos<N> &shape = Pos<N>::fill(1)) {
            return box_iterator(box, Box::presorted(box.min, box.min + shape - 1), shape);
        }
        static box_iterator end(const Box &box, const Pos<N> &shape = Pos<N>::fill(1)) {
            return box_iterator(box, None, shape);
        }

        box_iterator() : box_iterator(kUnitBox, None, Pos<N>::fill(1)) {}

        const Box &operator*() const { return current_.value(); }
        const Box *operator->() { return &current_.value(); }
        box_iterator &operator++() {
            if (current_.has_value()) {
                I64 i = N - 1;
                auto &current = current_.value();
                current.min[i] += shape_[i];
                current.max[i] += shape_[i];
                while (i >= 0 && current.max[i] > box_.max[i]) {
                    current.min[i] = box_.min[i];
                    current.max[i] = current.min[i] + shape_[i] - 1;
                    i -= 1;
                    if (i >= 0) {
                        current.min[i] += shape_[i];
                        current.max[i] += shape_[i];
                    } else {
                        current_ = None;
                    }
                }
            }
            return *this;
        }
        pure bool operator==(const box_iterator &rhs) const {
            return current_ == rhs.current_ && box_ == rhs.box_ && shape_ == rhs.shape_;
        }
        pure bool operator!=(const box_iterator &rhs) const { return !(*this == rhs); }

      private:
        explicit box_iterator(const Box &box, const Maybe<Box> &cur, const Pos<N> &shape)
            : box_(box), current_(cur), shape_(shape) {
            for (U64 i = 0; i < N; i++) {
                ASSERT(shape_[i] != 0, "Invalid iterator shape size of 0");
                ASSERT(shape_[i] > 0, "TODO: Support negative step");
            }
        }
        Box box_;
        Maybe<Box> current_;
        Pos<N> shape_;
    };

    explicit Box() = default;

    /// Returns a Box from points `a` to `b` (inclusive).
    /// Registers `min` and `max` fields to be the min and max in each dimension, respectively.
    constexpr Box(const Pos<N> &a, const Pos<N> &b) {
        for (U64 i = 0; i < N; i++) {
            min[i] = std::min(a[i], b[i]);
            max[i] = std::max(a[i], b[i]);
        }
    }

    /// Returns the number of dimensions in this box.
    pure constexpr I64 rank() const { return N; }

    pure Pos<N> shape() const { return max - min + 1; }

    /// Returns a new Box scaled by -1.
    pure Box operator-() const { return Box(-min, -max); }

    /// Returns a new Box scaled by the corresponding factors in `scale`.
    pure Box operator*(const Pos<N> &scale) const { return Box(min * scale, max * scale); }

    /// Returns a new Box scaled by the factor `scale` in every dimension.
    pure Box operator*(const I64 scale) const { return Box(min * scale, max * scale); }

    /// Returns a new Box shifted by `rhs`.
    pure Box operator+(const Pos<N> &rhs) const { return Box::presorted(min + rhs, max + rhs); }
    pure Box operator+(const I64 rhs) const { return Box::presorted(min + rhs, max + rhs); }
    pure Box operator-(const Pos<N> &rhs) const { return Box::presorted(min - rhs, max - rhs); }
    pure Box operator-(const I64 rhs) const { return Box::presorted(min - rhs, max - rhs); }

    /// Returns a new Box which is clamped to the given grid size.
    pure Box clamp(const Pos<N> &grid) const { return Box::presorted(min.clamp_down(grid), max.clamp_up(grid)); }
    pure Box clamp(const I64 grid) const { return Box::presorted(min.clamp_down(grid), max.clamp_up(grid)); }

    /// Returns an iterator over points in this box with the given `step` size in each dimension.
    pure Range<pos_iterator> pos_iter(const I64 step = 1) const {
        return Range<pos_iterator>(*this, Pos<N>::fill(step));
    }

    /// Returns an iterator over points in this box with the given multidimensional `step` size.
    pure Range<pos_iterator> pos_iter(const Pos<N> &step) const { return Range<pos_iterator>(*this, step); }

    /// Returns an iterator over sub-boxes with the given `step` size in each dimension.
    pure Range<box_iterator> box_iter(const I64 step) const { return Range<box_iterator>(*this, Pos<N>::fill(step)); }

    /// Returns an iterator over sub-boxes with the given multidimensional `step` size.
    pure Range<box_iterator> box_iter(const Pos<N> &shape) const { return Range<box_iterator>(*this, shape); }

    /// Provides iteration over all points in this box.
    pure pos_iterator begin() const { return pos_iterator::begin(*this); }
    pure pos_iterator end() const { return pos_iterator::end(*this); }

    pure bool operator==(const Box &rhs) const { return min == rhs.min && max == rhs.max; }
    pure bool operator!=(const Box &rhs) const { return !(*this == rhs); }

    /// Returns true if there is any overlap between this Box and `rhs`.
    pure bool overlaps(const Box &rhs) const {
        for (U64 i = 0; i < N; ++i) {
            if (min[i] > rhs.max[i] || max[i] < rhs.min[i]) {
                return false;
            }
        }
        return true;
    }

    /// Returns true if `pt` is somewhere within this box.
    pure bool contains(const Pos<N> &pt) const {
        for (U64 i = 0; i < N; ++i) {
            if (pt[i] < min[i] || pt[i] > max[i]) {
                return false;
            }
        }
        return true;
    }

    /// Returns the Box where this and `rhs` overlap. Returns None if there is no overlap.
    pure Maybe<Box> intersect(const Box &rhs) const {
        if (overlaps(rhs)) {
            return Box::presorted(nvl::max(min, rhs.min), nvl::min(max, rhs.max));
        }
        return None;
    }

    /// Returns the result of removing all points in `rhs` from this Box.
    /// This may result in anywhere between 1 and N*N - 1 boxes, depending on the nature of the intersection.
    pure List<Box> diff(const Box &rhs) const {
        List<Box> result;
        push_diff(result, rhs);
        return result;
    }

    pure List<Box> diff(const List<Box> &boxes) const {
        List<Box> result{*this};
        for (const Box &rhs : boxes) {
            List<Box> next;
            for (const Box &lhs : result) {
                lhs.push_diff(next, rhs);
            }
            result = next;
        }
        return result;
    }

    pure List<Edge<N>> edges(const I64 width = 1) const { return borders(-width, 0); }

    pure List<Edge<N>> borders(const I64 width = 1, const I64 dist = 1) const {
        List<Edge<N>> result;
        for (U64 i = 0; i < N; ++i) {
            for (const auto &dir : Dir::list) {
                result.push_back(border(i, dir, width, dist));
            }
        }
        return result;
    }

    pure Edge<N> border(const U64 dim, const Dir dir, const I64 width = 1, const I64 dist = 1) const {
        auto unit = Pos<N>::unit(dim);
        auto inner = unit * dist;
        auto outer = unit * (width - 1);
        auto border_min = (dir == Dir::Neg) ? min - outer : min.with(dim, max[dim]);
        auto border_max = (dir == Dir::Neg) ? max.with(dim, min[dim]) : max + outer;
        return Edge<N>(dim, dir, Box::presorted(border_min, border_max));
    }

    pure std::string to_string() const {
        std::stringstream ss;
        ss << min.to_string() << "::" << max.to_string();
        return ss.str();
    }

    Pos<N> min;
    Pos<N> max;

  private:
    friend class pos_iterator;

    // TODO: This is currently O(N^2), could be 2*N?
    void push_diff(List<Box> &result, const Box &rhs) const {
        if (const Maybe<Box> intersect = this->intersect(rhs)) {
            const Box &both = *intersect;
            for (const auto &pos : kUnitBox) {
                if (pos != Pos<N>::zero()) {
                    Pos<N> result_min;
                    Pos<N> result_max;
                    for (U64 d = 0; d < N; ++d) {
                        result_min[d] = (pos[d] == -1) ? min[d] : (pos[d] == 0) ? both.min[d] : both.max[d] + 1;
                        result_max[d] = (pos[d] == -1) ? both.min[d] - 1 : (pos[d] == 0) ? both.max[d] : max[d];
                    }
                    if (const Maybe<Box> box = Box::get(result_min, result_max)) {
                        result.push_back(*box);
                    }
                }
            }
        } else {
            result.push_back(*this);
        }
    }
};

template <U64 N>
constexpr Box<N> Box<N>::kUnitBox = Box(Pos<N>::fill(-1), Pos<N>::fill(1));

template <U64 N>
Box<N> operator*(I64 a, const Box<N> &b) {
    return b * a;
}
template <U64 N>
Box<N> operator+(I64 a, const Box<N> &b) {
    return b + a;
}
template <U64 N>
Box<N> operator-(I64 a, const Box<N> &b) {
    return -b + a;
}

template <U64 N>
std::ostream &operator<<(std::ostream &os, const Box<N> &box) {
    return os << box.to_string();
}

/// Returns the minimal Box which includes all of both Box `a` and `b`.
/// Note that the resulting area may be larger than the sum of the two areas.
template <U64 N>
pure Box<N> circumscribe(const Box<N> &a, const Box<N> &b) {
    // presorted is sufficient here since already taking the min/max across both
    return Box<N>::presorted(min(a.min, b.min), max(a.max, b.max));
}

} // namespace nvl

template <U64 N>
struct std::hash<nvl::Box<N>> {
    pure U64 operator()(const nvl::Box<N> &a) const { return nvl::sip_hash(a); }
};
