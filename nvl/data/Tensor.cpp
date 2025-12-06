#include "nvl/data/Tensor.h"

#include <fstream>

#include "nvl/data/List.h"
#include "nvl/macros/Aliases.h"

namespace nvl {

Tensor<2, char> matrix_from_lines(const List<std::string> &lines, const char empty) {
    const I64 rows = static_cast<I64>(lines.size());
    I64 cols = static_cast<I64>(lines[0].size());
    for (I64 i = 0; i < rows; ++i) {
        cols = std::max(cols, static_cast<I64>(lines[i].size()));
    }
    Tensor<2, char> matrix({rows, cols}, empty);
    for (I64 i = 0; i < rows; ++i) {
        for (I64 j = 0; j < static_cast<I64>(lines[i].size()); ++j) {
            matrix[{i, j}] = lines[i][j];
        }
    }
    return matrix;
}

Tensor<2, char> matrix_from_file(const std::string &filename, const char empty) {
    std::ifstream file(filename);
    std::string line;
    List<std::string> lines;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    return matrix_from_lines(lines, empty);
}

} // namespace nvl
