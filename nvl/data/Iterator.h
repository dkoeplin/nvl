#pragma once

namespace nvl {

template <typename Category, typename T, typename Distance = std::ptrdiff_t, typename Pointer = T *,
          typename Reference = T &>
struct Iterator {
    typedef T value_type;
    typedef Distance difference_type;
    typedef Pointer pointer;
    typedef Reference reference;
    typedef Category iterator_category;
};

} // namespace nvl
