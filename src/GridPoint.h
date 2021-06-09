#ifndef GRID_POINT_H
#define GRID_POINT_H

#include <array>
#include "TypeDefs.h"

struct GridPoint {
    std::array<TerminalIndex, num_dimensions> indices;

    GridPoint add(std::size_t coordinate, TerminalIndex offset) const {
        auto new_indices = indices;
        new_indices.at(coordinate) += offset;
        return { new_indices };
    }

    GridPoint subtract(std::size_t coordinate, TerminalIndex offset) const {
        auto new_indices = indices;
        new_indices.at(coordinate) -= offset;
        return { new_indices };
    }

    bool operator==(GridPoint const& other) const {
        return indices == other.indices;
    }

    GridPoint min(GridPoint const& other) const {
        GridPoint result;
        for (std::size_t i = 0; i < num_dimensions; ++i) {
            result.indices.at(i) = std::min(indices.at(i), other.indices.at(i));
        }
        return result;
    }

    GridPoint max(GridPoint const& other) const {
        GridPoint result;
        for (std::size_t i = 0; i < num_dimensions; ++i) {
            result.indices.at(i) = std::max(indices.at(i), other.indices.at(i));
        }
        return result;
    }
};

#endif
