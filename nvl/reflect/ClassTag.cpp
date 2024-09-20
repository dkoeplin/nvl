#include "nvl/macros/ReturnIf.h"
#include "nvl/reflect/ClassTag.h"

namespace nvl {

namespace {
constexpr bool is_subclass(const ClassTag &a, const ClassTag &b, const bool strict = false) {
    return_if(!strict && a == b, true);
    for (U64 i = 0; i < ClassTag::kMaxParents && a.parents[i] != nullptr; ++i) {
        return_if(is_subclass(*a.parents[i], b), true);
    }
    return false;
}
} // namespace

pure bool ClassTag::operator<=(const ClassTag &rhs) const { return is_subclass(*this, rhs); }
pure bool ClassTag::operator>=(const ClassTag &rhs) const { return is_subclass(rhs, *this); }
pure bool ClassTag::operator<(const ClassTag &rhs) const { return is_subclass(*this, rhs, true); }
pure bool ClassTag::operator>(const ClassTag &rhs) const { return is_subclass(rhs, *this, true); }

} // namespace nvl
