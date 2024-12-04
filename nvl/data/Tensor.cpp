#include "nvl/data/Tensor.h"

#include <fstream>

namespace nvl {

Tensor<2, char> matrix_from_file(const std::string &filename) {
    std::ifstream file(filename);

    std::string line;
    std::vector<std::string> lines;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    const int64_t rows = lines.size();
    const int64_t cols = lines[0].size();
    Tensor<2, char> matrix({rows, cols}, '.');
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            matrix[{i, j}] = lines[i][j];
        }
    }
    return matrix;
}

} // namespace nvl
