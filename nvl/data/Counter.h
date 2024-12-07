#pragma once

#include <utility>

#include "nvl/data/Iterator.h"
#include "nvl/data/List.h"
#include "nvl/data/Maybe.h"
#include "nvl/macros/Implicit.h"
#include "nvl/macros/Pure.h"

namespace nvl {

template <typename T>
class Counter {
public:
    using Value = List<T>;

    class iterator final : public AbstractIteratorCRTP<iterator, List<T>> {
    public:
        static Iterator<List<T>> begin(const Counter &ctr) { return make_iterator<iterator>(ctr, ctr->start_); }
        static Iterator<List<T>> end(const Counter &ctr) { return make_iterator<iterator>(ctr, None); }

        explicit iterator(const Counter &counter, Maybe<List<T>> iters) : ctr_(counter), iters_(std::move(iters)) {}

        void increment() override {
            // Increment only does something if this is not the end iterator.
            if (iters_.has_value()) {
                auto &idx = iters_.value();
                I64 i = iters_->size() - 1;
                idx[i] = idx[i] + ctr_->stride_[i];
                while (i >= 0 && (ctr_->stride_[i] > 0 ? idx[i] >= ctr_->end_[i] : idx[i] <= ctr_->end_[i])) {
                    idx[i] = ctr_->start_[i];
                    i -= 1;
                    if (i >= 0) {
                        idx[i] += ctr_->stride_[i];
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

    static Range<List<T>> get(const U64 n, const T ends) {
        return Counter(List<T>(n, 0), List<T>(n, ends), List<T>(n, 1)).range();
    }

    static Range<List<T>> get(const List<T> &end) {
        return Counter(List<T>(end.size(), 0), end_(end), List<T>(end.size(), 1)).range();
    }

    static Range<List<T>> get(const List<T> &start, const List<T> &end, const List<T> &stride) {
        return Counter(start, end, stride).range();
    }

    pure Iterator<List<T>> begin() const { return iterator::begin(this); }
    pure Iterator<List<T>> end() const { return iterator::end(this); }
    pure Range<List<T>> range() const { return make_range<iterator>(this); }

    pure U64 size() const { return start_.size(); }

    pure bool operator==(const Counter &rhs) const {
        return start_ == rhs.start_ && end_ == rhs.end_ && stride_ == rhs.stride_;
    }
    pure bool operator!=(const Counter &rhs) const { return !(*this == rhs); }

private:
    explicit Counter(const List<T> &start, const List<T> &end, const List<T> &stride)
        : start_(start), end_(end), stride_(stride) {}

    friend class iterator;
    List<T> start_;
    List<T> end_;
    List<T> stride_;
};

} // namespace nvl