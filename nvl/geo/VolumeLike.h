#pragma once

#include <string>

#include "nvl/data/Iterator.h"
#include "nvl/data/Maybe.h"
#include "nvl/data/Range.h"
#include "nvl/geo/Pos.h"
#include "nvl/geo/Vec.h"
#include "nvl/macros/Aliases.h"

namespace nvl {

template <U64 N, typename T>
struct Edge;

/**
 * @class Box
 * @brief An N-dimensional volume from [min, max).
 * @tparam N - The number of dimensions in this volume.
 */
template <U64 N, typename T, typename Pt, typename Concrete>
class VolumeLike {
public:
    /// Returns a Box with `min` and `max` if min is strictly less than or equal to max.
    /// Returns None otherwise.
    /// Used to detect cases e.g. where intersection results in an empty Box.
    static Maybe<Concrete> get(const Pt &min, const Pt &max) {
        if (min.all_lt(max)) {
            return Box(min, max);
        }
        return None;
    }

    struct pos_iterator final : AbstractIteratorCRTP<pos_iterator, Pt> {
        class_tag(VolumeLike::pos_iterator, AbstractIterator<Pt>);

        template <View Type = View::kImmutable>
            requires(Type == View::kImmutable)
        static Iterator<Pt, Type> begin(const Box &box, const Pt &step) {
            return make_iterator<pos_iterator>(box, box.min, step);
        }
        template <View Type = View::kImmutable>
            requires(Type == View::kImmutable)
        static Iterator<Pt, Type> end(const Box &box, const Pt &step) {
            return make_iterator<pos_iterator>(box, None, step);
        }

        pos_iterator() : pos_iterator(kUnitBox, None, Pt::fill(1)) {}
        explicit pos_iterator(const Box &box, const Maybe<Pt> &pos, const Pt &step)
            : box_(box), pos_(pos), step_(step) {
            for (U64 i = 0; i < N; i++) {
                ASSERT(step_[i] != 0, "Invalid iterator step size of 0");
                ASSERT(step_[i] > 0, "TODO: Support negative step");
            }
            if (pos_.has_value() && !box_.contains(*pos_))
                pos_ = None;
        }

        const Pt *ptr() override { return &pos_.value(); }

        pure bool operator==(const pos_iterator &rhs) const override {
            return pos_ == rhs.pos_ && box_ == rhs.box_ && step_ == rhs.step_;
        }

        void increment() override {
            // Increment only does something if this is not the end iterator.
            if (pos_.has_value()) {
                auto &pos = pos_.value();
                I64 i = N - 1;
                pos[i] = pos[i] + step_[i];
                while (i >= 0 && pos[i] >= box_.max[i]) {
                    pos[i] = box_.min[i];
                    i -= 1;
                    if (i >= 0) {
                        pos[i] += step_[i];
                    } else {
                        pos_ = None; // Reached end
                    }
                }
            }
        }

        Box<N> box_;
        Maybe<Pt> pos_;
        Pt step_;
    };

    struct box_iterator final : AbstractIteratorCRTP<box_iterator, Concrete> {
        class_tag(VolumeLike::box_iterator, AbstractIterator<Concrete>);

        template <View Type = View::kImmutable>
            requires(Type == View::kImmutable)
        static Iterator<Concrete, Type> begin(const Concrete &box, const Pt &shape = Pt::fill(1)) {
            return make_iterator<box_iterator, Type>(box, Concrete(box.min, box.min + shape), shape);
        }
        template <View Type = View::kImmutable>
            requires(Type == View::kImmutable)
        static Iterator<Concrete, Type> end(const Concrete &box, const Pt &shape = Pt::fill(1)) {
            return make_iterator<box_iterator, Type>(box, None, shape);
        }

        box_iterator() : box_iterator(kUnitBox, None, Pt::fill(1)) {}
        explicit box_iterator(const Concrete &box, const Maybe<Concrete> &cur, const Pt &shape)
            : box_(box), current_(cur), shape_(shape) {
            for (U64 i = 0; i < N; i++) {
                ASSERT(shape_[i] != 0, "Invalid iterator shape size of 0");
                ASSERT(shape_[i] > 0, "TODO: Support negative step");
            }
            if (current_.has_value() && !box_.overlaps(*current_))
                current_ = None;
        }

        const Concrete *ptr() override { return &current_.value(); }

        pure bool operator==(const box_iterator &rhs) const override {
            return current_ == rhs.current_ && box_ == rhs.box_ && shape_ == rhs.shape_;
        }

        void increment() override {
            if (current_.has_value()) {
                I64 i = N - 1;
                auto &current = current_.value();
                current.min[i] += shape_[i];
                current.max[i] += shape_[i];
                while (i >= 0 && current.max[i] > box_.max[i]) {
                    current.min[i] = box_.min[i];
                    current.max[i] = box_.min[i] + shape_[i];
                    i -= 1;
                    if (i >= 0) {
                        current.min[i] += shape_[i];
                        current.max[i] += shape_[i];
                    } else {
                        current_ = None;
                    }
                }
            }
        }

        Concrete box_;
        Maybe<Concrete> current_;
        Pt shape_;
    };

    explicit VolumeLike() = default;

    /// Returns a Box from points `a` to `b` (inclusive).
    /// Registers `min` and `max` fields to be the min and max in each dimension, respectively.
    constexpr VolumeLike(const Pt &a, const Pt &b) {
        for (U64 i = 0; i < N; i++) {
            min[i] = std::min(a[i], b[i]);
            max[i] = std::max(a[i], b[i]);
        }
    }

    /// Returns the number of dimensions in this box.
    pure constexpr I64 rank() const { return N; }

    pure Pt shape() const { return max - min; }

    pure bool empty() const { return min == max; }

    pure Concrete with(const U64 dim, const I64 lo, const I64 hi) const {
        return Concrete(this->min.with(dim, lo), this->max.with(dim, hi));
    }

    // TODO: Need to better define scaling by negatives here
    //   Scaling by -1: [1,3) => [-3,-1)? [-2,0)?
    //   Scaling by 2:  [1,3) => [2, 6)
    //   Scaling by -2: [1,3) => [-6, -2)? [-4, 0)?

    /// Returns a new Box scaled by -1.
    // pure Box operator-() const { return Box(-min, -max); }

    /// Returns a new Box scaled by the corresponding factors in `scale`.
    // pure Box operator*(const Pt &scale) const { return Box(min * scale, max * scale); }
    // pure Box operator*(const I64 scale) const { return Box(min * scale, max * scale); }
    // void operator*=(const Pos<2> &rhs) { *this = Box(min * rhs, max * rhs); }
    // void operator*=(const I64 &rhs) { *this = Box(min * rhs, max * rhs); }

    /// Returns a new Box shifted by `rhs`.
    pure Concrete operator+(const Pt &rhs) const { return Concrete(min + rhs, max + rhs); }
    pure Concrete operator+(const I64 rhs) const { return Concrete(min + rhs, max + rhs); }
    pure Concrete operator-(const Pt &rhs) const { return Concrete(min - rhs, max - rhs); }
    pure Concrete operator-(const I64 rhs) const { return Concrete(min - rhs, max - rhs); }

    void operator+=(const Pt &rhs) { *static_cast<Concrete *>(this) = Concrete(min + rhs, max + rhs); }
    void operator-=(const Pt &rhs) { *static_cast<Concrete *>(this) = Concrete(min - rhs, max - rhs); }
    void operator-=(const I64 &rhs) { *static_cast<Concrete *>(this) = Concrete(min - rhs, max - rhs); }
    void operator+=(const I64 &rhs) { *static_cast<Concrete *>(this) = Concrete(min + rhs, max + rhs); }

    /// Returns a new Box which is clamped to the given grid size.
    pure Concrete clamp(const Pt &grid) const { return Concrete(min.grid_min(grid), max.grid_max(grid)); }
    pure Concrete clamp(const I64 grid) const { return Concrete(min.grid_min(grid), max.grid_max(grid)); }

    /// Returns an iterator over points in this box with the given `step` size in each dimension.
    pure Range<Pt> pos_iter(const I64 step = 1) const { return pos_iter(Pt::fill(step)); }

    /// Returns an iterator over points in this box with the given multidimensional `step` size.
    pure Range<Pt> pos_iter(const Pt &step) const { return make_range<pos_iterator>(*this, step); }

    /// Returns an iterator over sub-boxes with the given `step` size in each dimension.
    pure Range<Concrete> box_iter(const I64 step) const { return make_range<box_iterator>(*this, Pt::fill(step)); }

    /// Returns an iterator over sub-boxes with the given multidimensional `step` size.
    pure Range<Concrete> box_iter(const Pt &shape) const { return make_range<box_iterator>(*this, shape); }

    /// Provides iteration over all points in this box.
    pure Iterator<Pt> begin() const { return pos_iterator::template begin(*this, Pt::ones); }
    pure Iterator<Pt> end() const { return pos_iterator::template end(*this, Pt::ones); }

    pure bool operator==(const Concrete &rhs) const { return min == rhs.min && max == rhs.max; }
    pure bool operator!=(const Concrete &rhs) const { return !(*this == rhs); }

    /// Returns true if there is any overlap between this Box and `rhs`.
    pure bool overlaps(const Concrete &rhs) const {
        for (U64 i = 0; i < N; ++i) {
            if (min[i] >= rhs.max[i] || rhs.min[i] >= max[i]) {
                return false;
            }
        }
        return true;
    }

    /// Returns true if `pt` is contained within this box.
    pure bool contains(const Pos<N> &pt) const {
        for (U64 i = 0; i < N; ++i) {
            if (pt[i] < min[i] || pt[i] >= max[i]) {
                return false;
            }
        }
        return true;
    }

    /// Returns true if `pt` is contained within this box.
    pure bool contains(const Vec<N> &pt) const {
        for (U64 i = 0; i < N; ++i) {
            if (pt[i] < min[i] || pt[i] >= max[i]) {
                return false;
            }
        }
        return true;
    }

    /// Returns the Box where this and `rhs` overlap. Returns None if there is no overlap.
    pure Maybe<Box> intersect(const Box &rhs) const {
        if (overlaps(rhs)) {
            return Box(nvl::max(min, rhs.min), nvl::min(max, rhs.max));
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

    template <typename Value>
        requires trait::HasBBox<Value>
    pure List<Concrete> diff(const Range<Value> &range) const {
        List<Concrete> result{*this};
        for (const auto &value : range) {
            List<Concrete> next;
            for (const Concrete &lhs : result) {
                lhs.push_diff(next, value.bbox());
            }
            result = next;
        }
        return result;
    }

    pure List<Concrete> diff(const List<Concrete> &boxes) const { return diff(Range(boxes.begin(), boxes.end())); }

    /// Returns the faces with given `width`.
    /// Faces begin at the outermost "pixel" of the box and extend inwards.
    pure List<Edge<N>> faces(I64 width = 1) const;

    /// Returns the edges with given width and distance from the outermost pixel.
    /// Edges begin at `dist` "pixels" away from the outermost 'pixel" of the box and extend outwards.
    pure List<Edge<N>> edges(I64 width = 1, I64 dist = 1) const;

    /// Returns the edge on the side of the box in dimension `dim` in direction `dir`.
    pure Edge<N> edge(Dir dir, U64 dim, I64 width = 1, I64 dist = 1) const;

    /// Returns a new Box which is expanded by `size` in every direction/dimension.
    /// e.g. [0,3) widened 3 => [-3,6)
    pure Concrete widened(const U64 size) const { return Concrete(min - Pt::fill(size), max + Pt::fill(size)); }

    pure std::string to_string() const {
        std::stringstream ss;
        ss << "{" << min.to_string() << ", " << max.to_string() << "}";
        return ss.str();
    }

    const Concrete &bbox() const { return *this; }

    Pt min;
    Pt max;

private:
    friend struct pos_iterator;

    void push_diff(List<Concrete> &result, const Concrete &rhs) const {
        const Maybe<Concrete> intersect = this->intersect(rhs);
        if (intersect.has_value()) {
            const Concrete &both = *intersect;
            for (U64 i = 0; i < N; ++i) {
                for (const Dir dir : Dir::list) {
                    Pt result_min;
                    Pt result_max;
                    for (U64 d = 0; d < N; ++d) {
                        if (i == d) {
                            result_min[d] = (dir == Dir::Neg) ? min[d] : both.max[d];
                            result_max[d] = (dir == Dir::Neg) ? both.min[d] : max[d];
                        } else if (d > i) {
                            result_min[d] = min[d];
                            result_max[d] = max[d];
                        } else {
                            result_min[d] = both.min[d];
                            result_max[d] = both.max[d];
                        }
                    }
                    const Maybe<Concrete> box = Concrete::get(result_min, result_max);
                    if (box.has_value()) {
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
struct Edge : Face {
    Edge() = default;
    explicit Edge(const Dir dir, const U64 dim, const Box<N> &box) : Face(dir, dim), box(box) {}

    pure List<Edge> diff(const Box<N> &rhs) const {
        List<Edge> result;
        for (const Box<N> &b : box.diff(rhs)) {
            result.emplace_back(dir, dim, b);
        }
        return result;
    }

    template <typename Value>
        requires trait::HasBBox<Value>
    pure List<Edge> diff(Range<Value> range) const {
        List<Edge> result;
        for (const Box<N> &b : box.diff(range)) {
            result.emplace_back(dir, dim, b);
        }
        return result;
    }

    pure bool operator==(const Edge &rhs) const { return dim == rhs.dim && dir == rhs.dir && box == rhs.box; }
    pure bool operator!=(const Edge &rhs) const { return !(*this == rhs); }

    pure U64 thickness() const { return box.shape(dim); }
    pure Box<N> bbox() const { return box; }

    pure Face face() const { return Face(dir, dim); }

    Box<N> box;
};

template <U64 N>
constexpr Box<N> Box<N>::kUnitBox = Box(Pt::fill(0), Pt::fill(1));

template <U64 N>
List<Edge<N>> Box<N>::faces(const I64 width) const {
    return edges(-width, 0);
}

template <U64 N>
List<Edge<N>> Box<N>::edges(const I64 width, const I64 dist) const {
    List<Edge<N>> result;
    for (U64 i = 0; i < N; ++i) {
        for (const auto &dir : Dir::list) {
            result.push_back(edge(dir, i, width, dist));
        }
    }
    return result;
}

template <U64 N>
Edge<N> Box<N>::edge(const Dir dir, const U64 dim, const I64 width, const I64 dist) const {
    auto unit = Pt::unit(dim);
    auto inner = unit * dist;
    auto outer = unit * (width - 1);
    auto edge_min = (dir == Dir::Neg) ? min - outer - inner : min.with(dim, max[dim]) + inner;
    auto edge_max = (dir == Dir::Neg) ? max.with(dim, min[dim]) - inner : max + outer + inner;
    return Edge<N>(dir, dim, Box(edge_min, edge_max));
}

/*template <U64 N>
Box<N> operator*(I64 a, const Box<N> &b) {
    return b * a;
}*/
template <U64 N>
Box<N> operator+(I64 a, const Box<N> &b) {
    return b + a;
}
/*template <U64 N>
Box<N> operator-(I64 a, const Box<N> &b) {
    return -b + a;
}*/

template <U64 N>
std::ostream &operator<<(std::ostream &os, const Box<N> &box) {
    return os << "{" << box.min << ", " << box.max << "}";
}

template <U64 N>
std::ostream &operator<<(std::ostream &os, const Edge<N> &edge) {
    return os << "Edge(" << edge.dir << edge.dim << ", " << edge.box << ")";
}

/// Returns the minimal Box which includes all of both Box `a` and `b`.
/// Note that the resulting area may be larger than the sum of the two areas.
template <U64 N>
pure Box<N> bounding_box(const Box<N> &a, const Box<N> &b) {
    return Box<N>(min(a.min, b.min), max(a.max, b.max));
}

} // namespace nvl

template <U64 N>
struct std::hash<nvl::Box<N>> {
    pure U64 operator()(const nvl::Box<N> &a) const noexcept { return nvl::sip_hash(a); }
};

template <U64 N>
struct std::hash<nvl::Edge<N>> {
    pure U64 operator()(const nvl::Edge<N> &a) const noexcept { return nvl::sip_hash(a); }
};