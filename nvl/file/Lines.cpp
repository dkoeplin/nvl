#include "nvl/file/Lines.h"

namespace nvl {

Lines::Lines(const std::string &filename) {
    std::ifstream infile(filename);
    next();
}

void Lines::next() {
    std::string next;
    if (std::getline(file, next)) {
        line = next;
    } else {
        line = None;
    }
}

} // namespace nvl
