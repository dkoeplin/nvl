#include "nvl/ui/Mouse.h"

namespace nvl {

std::string Mouse::to_string() const {
    static const std::string names[kNumButtons] = {"Left", "Right", "Middle", "Side", "Extra", "Forward", "Backward"};
    if (value >= 0 && value < kNumButtons)
        return names[value];
    return "Unknown (" + std::to_string(value) + ")";
}

} // namespace nvl