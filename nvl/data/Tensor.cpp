#include "nvl/data/Tensor.h"

#include <fstream>

#include "nvl/data/List.h"
#include "nvl/macros/Aliases.h"

namespace nvl {

Tensor<2, char> matrix_from_file(const std::string &filename) {
    std::ifstream file(filename);

    std::string line;
    List<std::string> lines;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    const I64 rows = lines.size();
    const I64 cols = lines[0].size();
    Tensor<2, char> matrix({rows, cols}, '.');
    for (I64 i = 0; i < rows; ++i) {
        for (I64 j = 0; j < cols; ++j) {
            matrix[{i, j}] = lines[i][j];
        }
    }
    return matrix;
}

} // namespace nvl
