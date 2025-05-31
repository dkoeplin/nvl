#pragma once

#include <fstream>
#include <string>

#include "nvl/data/Iterator.h"
#include "nvl/data/Maybe.h"
#include "nvl/macros/Pure.h"

namespace nvl {

/**
 * @class Lines
 * @brief An iterator over lines in a file.
 */
class Lines : public AbstractIteratorCRTP<Lines, std::string> {
public:
    explicit Lines(const std::string &filename);

    pure const std::string *ptr() override { return &line.value(); }
    pure bool operator==(const Lines &rhs) const override { return this == &rhs; }

protected:
    void next();
    void increment() override { next(); }
    std::ifstream file;
    Maybe<std::string> line;
};

} // namespace nvl
