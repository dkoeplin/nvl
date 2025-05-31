#pragma once

#include <utility>

#include "nvl/data/Iterator.h"
#include "nvl/data/List.h"
#include "nvl/data/Maybe.h"
#include "nvl/macros/Pure.h"

namespace nvl {

/**
 * @class Counter
 * @brief An N-dimensional iteration space with optional stride.
 * @tparam T - The numeric type to iterate over.
 */
template <typename T>
class Counter {
public:
    using Value = List<T>;
    class iterator;

    /**
     * Returns an [n]-dimensional counter, with each dimension counting from [0, ends) with a stride of 1.
     */
    static Range<List<T>> get(const U64 n, const T ends) {
        return Counter(List<T>(n, 0), List<T>(n, ends), List<T>(n, 1)).range();
    }

    /**
     * Returns an N-dimensional counter where each counter counts from [0, end_i) with a stride of 1.
     */
    static Range<List<T>> get(const List<T> &end) {
        return Counter(List<T>(end.size(), 0), end, List<T>(end.size(), 1)).range();
    }

    /**
     * Returns an N-dimensional counter with the given start, end, and stride per dimension.
     */
    static Range<List<T>> get(const List<T> &start, const List<T> &end, const List<T> &stride) {
        return Counter(start, end, stride).range();
    }

    pure Iterator<List<T>> begin() const;
    pure Iterator<List<T>> end() const;
    pure Range<List<T>> range() const;

    pure U64 rank() const { return start_.size(); }

    pure bool operator==(const Counter &rhs) const {
        return start_ == rhs.start_ && end_ == rhs.end_ && stride_ == rhs.stride_;
    }
    pure bool operator!=(const Counter &rhs) const { return !(*this == rhs); }

private:
    friend class iterator;

    explicit Counter(const List<T> &start, const List<T> &end, const List<T> &stride)
        : start_(start), end_(end), stride_(stride) {}

    List<T> start_;  /// Inclusive start per dimension
    List<T> end_;    /// Exclusive end per dimension
    List<T> stride_; /// Stride per dimension
};

/**
 * @class Counter::iterator
 * @brief An iterator over an N-dimensional iteration space.
 */
template <typename T>
class Counter<T>::iterator final : public AbstractIteratorCRTP<iterator, List<T>> {
public:
    class_tag(Counter<T>::iterator, AbstractIterator<List<T>>);

    template <View Type = View::kImmutable>
    static Iterator<List<T>, Type> begin(const Counter &ctr) {
        return make_iterator<iterator, Type>(ctr, ctr.start_);
    }

    template <View Type = View::kImmutable>
    static Iterator<List<T>, Type> end(const Counter &ctr) {
        return make_iterator<iterator, Type>(ctr, None);
    }

    explicit iterator(const Counter &counter, Maybe<List<T>> iters) : ctr_(counter), iters_(std::move(iters)) {}

    void increment() override {
        // Increment only does something if this is not the end iterator.
        if (iters_.has_value()) {
            auto &idx = iters_.value();
            I64 i = iters_->size() - 1;
            idx[i] = idx[i] + ctr_.stride_[i];
            while (i >= 0 && (ctr_.stride_[i] > 0 ? idx[i] >= ctr_.end_[i] : idx[i] <= ctr_.end_[i])) {
                idx[i] = ctr_.start_[i];
                i -= 1;
                if (i >= 0) {
                    idx[i] += ctr_.stride_[i];
                } else {
                    iters_ = None; // Reached end
                }
            }
        }
    }

    pure const List<T> *ptr() override { return &iters_.value(); }
    pure bool operator==(const iterator &rhs) const override { return ctr_ == rhs.ctr_ && iters_ == rhs.iters_; }

private:
    Counter ctr_;
    Maybe<List<T>> iters_;
};

template <typename T>
pure Iterator<List<T>> Counter<T>::begin() const {
    return iterator::begin(*this);
}

template <typename T>
pure Iterator<List<T>> Counter<T>::end() const {
    return iterator::end(*this);
}

template <typename T>
pure Range<List<T>> Counter<T>::range() const {
    return make_range<iterator>(*this);
}

} // namespace nvl