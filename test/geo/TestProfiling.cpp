#include <fstream>

#include "gtest/gtest.h"
#include "nvl/data/Map.h"
#include "nvl/data/Tensor.h"
#include "nvl/entity/Block.h"
#include "nvl/geo/Volume.h"
#include "nvl/macros/Aliases.h"
#include "nvl/time/Clock.h"
#include "nvl/time/Duration.h"

namespace {
using namespace nvl;

pure U64 num_sides(const Range<Rel<Edge<2, I64>>> &edges) {
    U64 sides = 0;
    Map<Face, RTree<2, Edge<2, I64>>> faces;
    // Hash by which face (direction)
    for (const auto &edge : edges) {
        faces[edge->face()].insert(edge);
    }
    // Find connected components within each of those
    for (const auto &tree : faces.values()) {
        sides += tree.components().size();
    }
    return sides;
}

TEST(TestProfiling, profile_components) {
    const auto map = matrix_from_file("../../../data/12");
    const auto start = Clock::now();
    Map<char, RTree<2, Box<2>, Rel<Box<2>>>> plots;
    for (const auto i : map.indices()) {
        auto &list = plots[map[i]];
        list.emplace(i, i + 1);
    }
    U64 part1 = 0;
    U64 part2 = 0;
    for (const auto &tree : plots.values()) {
        for (const auto &component : tree.components()) {
            const BRTree<2, Box<2>> edges(component.values());
            const U64 area = component.size();
            const U64 perimeter = edges.edges().size();
            const U64 sides = num_sides(edges.edges());
            part1 += area * perimeter;
            part2 += area * sides;
        }
    }
    const auto end = Clock::now();
    std::cout << "Part 1: " << part1 << std::endl;
    std::cout << "Part 2: " << part2 << std::endl;
    std::cout << "Time: " << Duration(end - start) << std::endl;
}

} // namespace
