#include "nvl/ui/Color.h"

#include <sstream>

namespace nvl {

pure std::string Color::to_string() const {
    std::stringstream ss;
    ss << std::hex;
    ss << "0x" << r << "|" << g << "|" << b << "|" << a;
    return ss.str();
}

} // namespace nvl