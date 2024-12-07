#pragma once

#include <utility>

#include "nvl/data/Iterator.h"
#include "nvl/data/List.h"
#include "nvl/data/Maybe.h"
#include "nvl/macros/Implicit.h"
#include "nvl/macros/Pure.h"

namespace nvl {

class Counter {
public:
    using Value = List<I64>;

    class iterator final : public AbstractIteratorCRTP<iterator, List<I64>> {
    public:
        static Iterator<List<I64>> begin(const Counter *ctr) { return make_iterator<iterator>(ctr, ctr->start_); }
        static Iterator<List<I64>> end(const Counter *ctr) { return make_iterator<iterator>(ctr, None); }

        explicit iterator(const Counter *counter, Maybe<List<I64>> iters) : ctr_(counter), iters_(std::move(iters)) {}

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

        pure const List<I64> *ptr() override { return &iters_.value(); }

        pure bool operator==(const iterator &rhs) const override { return *ctr_ == *rhs.ctr_ && iters_ == rhs.iters_; }

    private:
        const Counter *ctr_;
        Maybe<List<I64>> iters_;
    };

    explicit Counter(const nvl::List<I64> &end) : start_(end.size(), 0), end_(end), stride_(end.size(), 1) {}

    explicit Counter(const nvl::List<I64> &start, const nvl::List<I64> &end, const nvl::List<I64> &stride)
        : start_(start), end_(end), stride_(stride) {}

    pure implicit operator Range<List<I64>>() const { return range(); }

    pure Iterator<List<I64>> begin() const { return iterator::begin(this); }
    pure Iterator<List<I64>> end() const { return iterator::end(this); }
    pure Range<List<I64>> range() const { return make_range<iterator>(this); }

    pure U64 size() const { return start_.size(); }

    pure bool operator==(const Counter &rhs) const {
        return start_ == rhs.start_ && end_ == rhs.end_ && stride_ == rhs.stride_;
    }
    pure bool operator!=(const Counter &rhs) const { return !(*this == rhs); }

private:
    friend class iterator;
    nvl::List<I64> start_;
    nvl::List<I64> end_;
    nvl::List<I64> stride_;
};

} // namespace nvl