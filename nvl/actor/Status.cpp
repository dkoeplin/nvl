#include "nvl/actor/Status.h"

#include "nvl/data/Map.h"

namespace nvl {

std::ostream &operator<<(std::ostream &os, const Status status) {
    static const Map<Status, std::string_view> map{
        {Status::kNone, "None"}, {Status::kIdle, "Idle"}, {Status::kMove, "Move"}, {Status::kDied, "Died"}};
    return os << map.get_or(status, "?");
}

} // namespace nvl
