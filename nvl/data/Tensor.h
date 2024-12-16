#pragma once

#include "nvl/data/List.h"
#include "nvl/geo/Tuple.h"
#include "nvl/geo/Volume.h"
#include "nvl/macros/Aliases.h"
#include "nvl/macros/Assert.h"
#include "nvl/macros/Expand.h"
#include "nvl/macros/Pure.h"
#include "nvl/macros/ReturnIf.h"

namespace nvl {

template <U64 N, typename T>
class Tensor {
public:
    using Idx = Tuple<N, I64>;

    /// Creates an empty tuple of empty shape.
    Tensor() : shape_(Idx::zero), strides_(Idx::zero) {}

    explicit Tensor(Idx shape, T init) : shape_(shape), data_(shape_.product(), init) { strides_ = shape_.strides(); }

    pure MIterator<T> begin() { return data_.begin(); }
    pure MIterator<T> end() { return data_.end(); }
    pure Iterator<T> begin() const { return data_.begin(); }
    pure Iterator<T> end() const { return data_.end(); }

    pure Range<Idx> indices() const { return Volume<N, I64>(Idx::zero, shape_).indices(); }

    /// Returns the first index where the given predicate is true. Returns None otherwise.
    pure Maybe<Idx> index_where(const std::function<bool(T)> &predicate) const {
        for (const auto &idx : indices()) {
            return_if(predicate((*this)[idx]), idx);
        }
        return None;
    }

    pure expand bool has(Idx indices) const { return indices.all_gte(Idx::zero) && indices.all_lt(shape_); }

    pure T &operator[](Idx indices) { return data_[flatten_index(indices)]; }
    pure const T &operator[](Idx indices) const { return data_[flatten_index(indices)]; }

    pure Maybe<T> get(Idx indices) const { return has(indices) ? Some(operator[](indices)) : None; }
    pure const T &get_or(Idx indices, const T &els) const { return has(indices) ? operator[](indices) : els; }

    pure Idx shape() const { return shape_; }
    pure Idx strides() const { return strides_; }
    pure U64 rank() const { return N; }

    pure bool operator==(const Tensor &rhs) const { return shape_ == rhs.shape_ && data_ == rhs.data_; }
    pure bool operator!=(const Tensor &rhs) const { return !(*this == rhs); }

private:
    pure expand I64 flatten_index(Idx indices) const {
        ASSERT(has(indices), "Invalid indices " << indices << " for tensor shape " << shape_);
        return (indices * strides_).sum();
    }
    Idx shape_;
    List<T> data_;
    Idx strides_;
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
            if (mismatches >= max_mismatches) {
                os << "(Hit maximum number of mismatches: " << max_mismatches << ")" << std::endl;
                return false;
            }
        }
    }
    return mismatches == 0;
}

Tensor<2, char> matrix_from_lines(const List<std::string> &lines);

Tensor<2, char> matrix_from_file(const std::string &filename);

} // namespace nvl
