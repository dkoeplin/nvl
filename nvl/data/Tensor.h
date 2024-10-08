#pragma once

#include "nvl/data/List.h"
#include "nvl/geo/Box.h"
#include "nvl/geo/Pos.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/Assert.h"
#include "nvl/macros/Expand.h"
#include "nvl/macros/Pure.h"

namespace nvl {

template <U64 N, typename T>
class Tensor {
public:
    explicit Tensor(Pos<N> shape, T init) : shape_(shape), data_(shape_.product(), init) {
        strides_ = shape_.strides();
    }

    pure MIterator<T> begin() { return data_.begin(); }
    pure MIterator<T> end() { return data_.end(); }
    pure Iterator<T> begin() const { return data_.begin(); }
    pure Iterator<T> end() const { return data_.end(); }

    pure Range<Pos<N>> indices() const { return Box(Pos<N>::zero, shape_ - 1).pos_iter(); }

    pure expand bool has(Pos<N> indices) const { return indices.all_gte(Pos<N>::zero) && indices.all_lt(shape_); }

    pure T &operator[](Pos<N> indices) { return data_[flatten_index(indices)]; }
    pure const T &operator[](Pos<N> indices) const { return data_[flatten_index(indices)]; }

    pure Maybe<T> get(Pos<N> indices) const { return has(indices) ? Some(operator[](indices)) : None; }
    pure const T &get_or(Pos<N> indices, const T &els) const { return has(indices) ? operator[](indices) : els; }

    pure Pos<N> shape() const { return shape_; }
    pure Pos<N> strides() const { return strides_; }
    pure U64 rank() const { return N; }

    pure bool operator==(const Tensor &rhs) const { return shape_ == rhs.shape_ && data_ == rhs.data_; }
    pure bool operator!=(const Tensor &rhs) const { return !(*this == rhs); }

private:
    pure expand I64 flatten_index(Pos<N> indices) const {
        ASSERT(has(indices), "Invalid indices " << indices << " for tensor shape " << shape_);
        return (indices * strides_).sum();
    }
    Pos<N> shape_;
    List<T> data_;
    Pos<N> strides_;
};

template <U64 N, typename T>
bool compare_tensors(std::ostream &os, const Tensor<N, T> &a, const Tensor<N, T> &b, const U64 max_mismatches = 5) {
    if (a.shape() != b.shape()) {
        os << "Size mismatch: " << a.shape() << " != " << b.shape() << std::endl;
        return false;
    }
    U64 mismatches = 0;
    for (auto i : a.indices()) {
        if (a[i] != b[i]) {
            os << "Mismatch at " << i << ": " << a[i] << " != " << b[i] << std::endl;
            mismatches += 1;
            if (mismatches >= max_mismatches)
                return false;
        }
    }
    return mismatches == 0;
}

} // namespace nvl
