#pragma once

#include <cmath>

#include "nvl/data/Counter.h"
#include "nvl/data/Iterator.h"
#include "nvl/data/List.h"
#include "nvl/data/Maybe.h"
#include "nvl/data/Range.h"
#include "nvl/geo/Dir.h"
#include "nvl/geo/Face.h"
#include "nvl/geo/HasBBox.h"
#include "nvl/geo/Tuple.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/Pure.h"

namespace nvl {

template <U64 N, typename T>
struct Edge;

/**
 * @class Volume
 * @brief An N-dimensional volume from [min, max).
 * @tparam N - The number of dimensions in this volume.
 * @tparam T - The type of elements holding the minimum and maximum.
 */
template <U64 N, typename T>
class Volume {
public:
    using Idx = Tuple<N, T>;

    /// Returns a Volume with `min` and `max` if min is strictly less than or equal to max.
    /// Returns None otherwise.
    /// Used to detect cases e.g. where intersection results in an empty Volume.
    static Maybe<Volume> get(const Idx &min, const Idx &max) {
        if (min.all_lt(max)) {
            return Volume(min, max);
        }
        return None;
    }

    /// Returns a volume with exactly one point.
    /// Only available for integer volumes.
    static Volume unit(const Idx &pt)
        requires std::is_same_v<T, I64>
    {
        return Volume(pt, pt + Idx::ones);
    }

    static const Volume kEmpty;

    struct idx_iterator final : AbstractIteratorCRTP<idx_iterator, Idx> {
        class_tag(Volume::idx_iterator, AbstractIterator<Idx>);

        template <View Type = View::kImmutable>
            requires(Type == View::kImmutable)
        static Iterator<Idx, Type> begin(const Volume &box, const Idx &step) {
            return make_iterator<idx_iterator>(box, box.min, step);
        }
        template <View Type = View::kImmutable>
            requires(Type == View::kImmutable)
        static Iterator<Idx, Type> end(const Volume &box, const Idx &step) {
            return make_iterator<idx_iterator>(box, None, step);
        }

        explicit idx_iterator(const Volume &box, const Maybe<Idx> &idx, const Idx &step)
            : box_(box), idx_(idx), step_(step) {
            for (U64 i = 0; i < N; i++) {
                ASSERT(step_[i] != 0, "Invalid iterator step size of 0");
                ASSERT(step_[i] > 0, "TODO: Support negative step");
            }
            if (idx_.has_value() && !box_.contains(*idx_))
                idx_ = None;
        }

        const Idx *ptr() override { return &idx_.value(); }

        pure bool operator==(const idx_iterator &rhs) const override {
            return idx_ == rhs.idx_ && box_ == rhs.box_ && step_ == rhs.step_;
        }

        void increment() override {
            // Increment only does something if this is not the end iterator.
            if (idx_.has_value()) {
                auto &idx = idx_.value();
                I64 i = N - 1;
                idx[i] = idx[i] + step_[i];
                while (i >= 0 && idx[i] >= box_.end[i]) {
                    idx[i] = box_.min[i];
                    i -= 1;
                    if (i >= 0) {
                        idx[i] += step_[i];
                    } else {
                        idx_ = None; // Reached end
                    }
                }
            }
        }

        Volume box_;
        Maybe<Idx> idx_;
        Idx step_;
    };

    struct box_iterator final : AbstractIteratorCRTP<box_iterator, Volume> {
        class_tag(Volume::box_iterator, AbstractIterator<Volume<N, T>>);

        template <View Type = View::kImmutable>
            requires(Type == View::kImmutable)
        static Iterator<Volume, Type> begin(const Volume &box, const Idx &shape = Idx::fill(1)) {
            return make_iterator<box_iterator, Type>(box, Volume(box.min, box.min + shape), shape);
        }
        template <View Type = View::kImmutable>
            requires(Type == View::kImmutable)
        static Iterator<Volume, Type> end(const Volume &box, const Idx &shape = Idx::fill(1)) {
            return make_iterator<box_iterator, Type>(box, None, shape);
        }

        explicit box_iterator(const Volume &box, const Maybe<Volume> &cur, const Idx &shape)
            : box_(box), current_(cur), shape_(shape) {
            for (U64 i = 0; i < N; i++) {
                ASSERT(shape_[i] != 0, "Invalid iterator shape size of 0");
                ASSERT(shape_[i] > 0, "TODO: Support negative step");
            }
            if (current_.has_value() && !box_.overlaps(*current_))
                current_ = None;
        }

        const Volume *ptr() override { return &current_.value(); }

        pure bool operator==(const box_iterator &rhs) const override {
            return current_ == rhs.current_ && box_ == rhs.box_ && shape_ == rhs.shape_;
        }

        void increment() override {
            if (current_.has_value()) {
                I64 i = N - 1;
                auto &current = current_.value();
                current.min[i] += shape_[i];
                current.end[i] += shape_[i];
                while (i >= 0 && current.end[i] > box_.end[i]) {
                    current.min[i] = box_.min[i];
                    current.end[i] = box_.min[i] + shape_[i];
                    i -= 1;
                    if (i >= 0) {
                        current.min[i] += shape_[i];
                        current.end[i] += shape_[i];
                    } else {
                        current_ = None;
                    }
                }
            }
        }

        Volume box_;
        Maybe<Volume> current_;
        Idx shape_;
    };

    struct face_iterator;

    explicit Volume() = default;

    /// Returns a Volume from points `a` to `b` (inclusive).
    /// Registers `min` and `max` fields to be the min and max in each dimension, respectively.
    constexpr Volume(const Idx &a, const Idx &b) {
        for (U64 i = 0; i < N; i++) {
            min[i] = std::min(a[i], b[i]);
            end[i] = std::max(a[i], b[i]);
        }
    }

    /// Returns the number of dimensions in this box.
    pure constexpr I64 rank() const { return N; }

    pure Idx shape() const { return end - min; }

    pure bool empty() const { return min == end; }

    pure Volume with(const U64 dim, const I64 lo, const I64 hi) const {
        return Volume(this->min.with(dim, lo), this->end.with(dim, hi));
    }

    // TODO: Need to better define scaling by negatives here
    //   Scaling by -1: [1,3) => [-3,-1)
    //   Scaling by 2:  [1,3) => [2, 6)
    //   Scaling by -2: [1,3) => [-6, -2)

    /// Returns a new Volume shifted by `rhs`.
    pure Volume operator+(const Idx &rhs) const { return Volume(min + rhs, end + rhs); }
    pure Volume operator+(const I64 rhs) const { return Volume(min + rhs, end + rhs); }
    pure Volume operator-(const Idx &rhs) const { return Volume(min - rhs, end - rhs); }
    pure Volume operator-(const I64 rhs) const { return Volume(min - rhs, end - rhs); }

    void operator+=(const Idx &rhs) { *this = Volume(min + rhs, end + rhs); }
    void operator-=(const Idx &rhs) { *this = Volume(min - rhs, end - rhs); }
    void operator-=(const I64 &rhs) { *this = Volume(min - rhs, end - rhs); }
    void operator+=(const I64 &rhs) { *this = Volume(min + rhs, end + rhs); }

    /// Returns a new Volume which is clamped to the given grid size.
    pure Volume clamp(const Idx &grid) const { return Volume(min.grid_min(grid), end.grid_max(grid)); }
    pure Volume clamp(const I64 grid) const { return Volume(min.grid_min(grid), end.grid_max(grid)); }

    /// Returns an iterator over points in this box with the given `step` size in each dimension.
    pure Range<Idx> indices(const I64 step = 1) const { return indices(Idx::fill(step)); }

    /// Returns an iterator over points in this box with the given multidimensional `step` size.
    pure Range<Idx> indices(const Idx &step) const { return make_range<idx_iterator>(*this, step); }

    /// Returns an iterator over sub-boxes with the given `step` size in each dimension.
    pure Range<Volume> volumes(const I64 step) const { return make_range<box_iterator>(*this, Idx::fill(step)); }

    /// Returns an iterator over sub-boxes with the given multidimensional `step` size.
    pure Range<Volume> volumes(const Idx &shape) const { return make_range<box_iterator>(*this, shape); }

    pure bool operator==(const Volume &rhs) const { return min == rhs.min && end == rhs.end; }
    pure bool operator!=(const Volume &rhs) const { return !(*this == rhs); }

    /// Returns true if there is any overlap between this Volume and `rhs`.
    pure bool overlaps(const Volume &rhs) const {
        for (U64 i = 0; i < N; ++i) {
            if (min[i] >= rhs.end[i] || rhs.min[i] >= end[i]) {
                return false;
            }
        }
        return true;
    }

    /// Returns true if `pt` is contained within this box.
    template <typename R>
    pure bool contains(const Tuple<N, R> &pt) const {
        for (U64 i = 0; i < N; ++i) {
            if (pt[i] < min[i] || pt[i] >= end[i]) {
                return false;
            }
        }
        return true;
    }

    /// Returns the Volume where this and `rhs` overlap. Returns None if there is no overlap.
    pure Maybe<Volume> intersect(const Volume &rhs) const {
        if (overlaps(rhs)) {
            return Volume(nvl::max(min, rhs.min), nvl::min(end, rhs.end));
        }
        return None;
    }

    /// Returns the result of removing all points in `rhs` from this Volume.
    /// This may result in anywhere between 1 and N*N - 1 boxes, depending on the nature of the intersection.
    pure List<Volume> diff(const Volume &rhs) const {
        List<Volume> result;
        push_diff(result, rhs);
        return result;
    }

    template <typename Value>
        requires trait::HasBBox<Value>
    pure List<Volume> diff(const Range<Value> &range) const {
        List<Volume> result{*this};
        for (const auto &value : range) {
            List<Volume> next;
            for (const Volume &lhs : result) {
                lhs.push_diff(next, value.bbox());
            }
            result = next;
        }
        return result;
    }

    pure List<Volume> diff(const List<Volume> &boxes) const { return diff(Range(boxes.begin(), boxes.end())); }

    /// Returns a range over the faces of this volume.
    /// Faces have a "thickness" of zero.
    pure Range<Edge<N, T>> faces() const;

    /// Returns the edges with given width and distance from the outermost pixel.
    /// Edges begin at `dist` "pixels" away from the outermost 'pixel" of the box and extend outwards.
    pure List<Edge<N, T>> edges(I64 width = 1, I64 dist = 0) const;

    /// Returns the edge on the side of the box in dimension `dim` in direction `dir`.
    pure Edge<N, T> edge(Dir dir, U64 dim, I64 width = 1, I64 dist = 0) const;

    /// Returns a new Volume which is expanded by `size` in every direction/dimension.
    /// e.g. [0,3) widened 3 => [-3,6)
    pure Volume widened(const U64 size) const { return Volume(min - Idx::fill(size), end + Idx::fill(size)); }

    pure std::string to_string() const {
        std::stringstream ss;
        ss << "{" << min.to_string() << ", " << end.to_string() << "}";
        return ss.str();
    }

    const Volume &bbox() const { return *this; }

    /// Returns the maximum value representable in F64 per dimension.
    pure Vec<N> max_f64() const {
        Vec<N> result;
        for (U64 i = 0; i < N; ++i) {
            result[i] = std::nextafter(static_cast<F64>(end[i]), -std::numeric_limits<F64>::infinity());
        }
        return result;
    }

    Idx min;
    Idx end;

private:
    friend struct idx_iterator;

    void push_diff(List<Volume> &result, const Volume &rhs) const {
        const Maybe<Volume> intersect = this->intersect(rhs);
        if (intersect.has_value()) {
            const Volume &both = *intersect;
            for (U64 i = 0; i < N; ++i) {
                for (const Dir dir : Dir::list) {
                    Tuple<N, T> result_min;
                    Tuple<N, T> result_end;
                    for (U64 d = 0; d < N; ++d) {
                        if (i == d) {
                            result_min[d] = (dir == Dir::Neg) ? min[d] : both.end[d];
                            result_end[d] = (dir == Dir::Neg) ? both.min[d] : end[d];
                        } else if (d > i) {
                            result_min[d] = min[d];
                            result_end[d] = end[d];
                        } else {
                            result_min[d] = both.min[d];
                            result_end[d] = both.end[d];
                        }
                    }
                    const Maybe<Volume> box = Volume::get(result_min, result_end);
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

template <U64 N, typename T>
const Volume<N, T> Volume<N, T>::kEmpty = Volume(Tuple<N, T>::zero, Tuple<N, T>::zero);

template <U64 N>
using Box = Volume<N, I64>;

template <U64 N, typename T>
struct Edge : Face {
    Edge() = default;
    explicit Edge(const Dir dir, const U64 dim, const Volume<N, T> &box) : Face(dir, dim), box(box) {}

    pure List<Edge> diff(const Volume<N, T> &rhs) const {
        List<Edge> result;
        for (const Volume<N, T> &b : box.diff(rhs)) {
            result.emplace_back(dir, dim, b);
        }
        return result;
    }

    template <typename Value>
        requires trait::HasBBox<Value>
    pure List<Edge> diff(Range<Value> range) const {
        List<Edge> result;
        for (const Volume<N, T> &b : box.diff(range)) {
            result.emplace_back(dir, dim, b);
        }
        return result;
    }

    pure bool operator==(const Edge &rhs) const { return dim == rhs.dim && dir == rhs.dir && box == rhs.box; }
    pure bool operator!=(const Edge &rhs) const { return !(*this == rhs); }

    pure U64 thickness() const { return box.shape(dim); }
    pure Volume<N, T> bbox() const { return box; }

    pure Face face() const { return Face(dir, dim); }

    Volume<N, T> box;
};

template <U64 N, typename T>
struct Volume<N, T>::face_iterator final : AbstractIteratorCRTP<face_iterator, Edge<N, T>> {
    class_tag(Volume::face_iterator, AbstractIterator<Edge<N, T>>);

    template <View Type = View::kImmutable>
        requires(Type == View::kImmutable)
    static Iterator<Edge<N, T>, Type> begin(const Volume &box) {
        return make_iterator<face_iterator, Type>(box, Dir::Neg, 0);
    }
    template <View Type = View::kImmutable>
        requires(Type == View::kImmutable)
    static Iterator<Edge<N, T>, Type> end(const Volume &box) {
        return make_iterator<face_iterator, Type>(box, Dir::Neg, N);
    }

    explicit face_iterator(const Volume &box, Dir dir, U64 dim) : box_(box), face_(dir, dim, box) { update_face(); }

    const Edge<N, T> *ptr() override { return &face_; }

    pure bool operator==(const face_iterator &rhs) const override {
        return box_ == rhs.box_ && face_.dim == rhs.face_.dim && face_.dir == rhs.face_.dir;
    }

    void increment() override {
        if (face_.dim < N) {
            if (face_.dir == Dir::Neg) {
                face_.dir = Dir::Pos;
            } else {
                face_.dir = Dir::Neg;
                face_.dim += 1;
            }
            update_face();
        }
    }

    void update_face() {
        const U64 i = face_.dim;
        return_if(i >= N);
        face_.box.min = (face_.dir == Dir::Neg) ? box_.min : box_.min.with(i, box_.end[i]);
        face_.box.end = (face_.dir == Dir::Neg) ? box_.end.with(i, box_.min[i]) : box_.end;
    }

    Volume box_;
    Edge<N, T> face_;
};

template <U64 N, typename T>
Range<Edge<N, T>> Volume<N, T>::faces() const {
    return make_range<face_iterator>(*this);
}

template <U64 N, typename T>
List<Edge<N, T>> Volume<N, T>::edges(const I64 width, const I64 dist) const {
    List<Edge<N, T>> result;
    for (U64 i = 0; i < N; ++i) {
        for (const auto &dir : Dir::list) {
            result.push_back(edge(dir, i, width, dist));
        }
    }
    return result;
}

template <U64 N, typename T>
Edge<N, T> Volume<N, T>::edge(const Dir dir, const U64 dim, const I64 width, const I64 dist) const {
    auto unit = Tuple<N, T>::unit(dim);
    auto inner = unit * dist;
    auto outer = unit * width;
    auto edge_min = (dir == Dir::Neg) ? min - outer - inner : min.with(dim, end[dim]) + inner;
    auto edge_end = (dir == Dir::Neg) ? end.with(dim, min[dim]) - inner : end + outer + inner;
    return Edge<N, T>(dir, dim, Volume(edge_min, edge_end));
}

template <U64 N, typename T>
Volume<N, T> operator+(I64 a, const Volume<N, T> &b) {
    return b + a;
}

template <U64 N, typename T>
std::ostream &operator<<(std::ostream &os, const Volume<N, T> &box) {
    return os << "{" << box.min << ", " << box.end << "}";
}

template <U64 N, typename T>
std::ostream &operator<<(std::ostream &os, const Edge<N, T> &edge) {
    return os << "Edge(" << edge.dir << edge.dim << ", " << edge.box << ")";
}

/// Returns the minimal Volume which includes all of both Volume `a` and `b`.
/// Note that the resulting area may be larger than the sum of the two areas.
template <U64 N, typename T>
pure Volume<N, T> bounding_box(const Volume<N, T> &a, const Volume<N, T> &b) {
    if (a.empty())
        return b;
    if (b.empty())
        return a;
    return Volume<N, T>(min(a.min, b.min), max(a.end, b.end));
}

} // namespace nvl

template <U64 N, typename T>
struct std::hash<nvl::Volume<N, T>> {
    pure U64 operator()(const nvl::Volume<N, T> &a) const noexcept { return nvl::sip_hash(a); }
};

template <U64 N, typename T>
struct std::hash<nvl::Edge<N, T>> {
    pure U64 operator()(const nvl::Edge<N, T> &a) const noexcept { return nvl::sip_hash(a); }
};
