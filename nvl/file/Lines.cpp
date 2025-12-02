#include "nvl/file/Lines.h"

namespace nvl {

Lines::Lines(const std::string &filename) {
    file = std::make_shared<std::fstream>(filename);
    next();
}

void Lines::next() {
    std::string next;
    if (std::getline(*file, next)) {
        line = next;
    } else {
        line = None;
    }
}

} // namespace nvl
