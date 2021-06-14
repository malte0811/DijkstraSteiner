#ifndef GRID_POINT_H
#define GRID_POINT_H

#include <array>
#include "TypeDefs.h"

class HananGrid;

struct GridPoint {
    using Coordinates = std::array<TerminalIndex, num_dimensions>;

    Coordinates indices;
    VertexIndex global_index;

    GridPoint next(std::size_t coordinate, VertexIndex axis_factor) const {
        auto new_indices = indices;
        ++new_indices.at(coordinate);
        return {new_indices, static_cast<VertexIndex>(global_index + axis_factor)};
    }

    GridPoint previous(std::size_t coordinate, VertexIndex axis_factor) const {
        auto new_indices = indices;
        --new_indices.at(coordinate);
        return {new_indices, static_cast<VertexIndex>(global_index - axis_factor)};
    }

    bool operator==(GridPoint const& other) const {
        return indices == other.indices;
    }

    Coordinates min(Coordinates const& other) const {
        Coordinates result;
        for (std::size_t i = 0; i < num_dimensions; ++i) {
            result.at(i) = std::min(indices.at(i), other.at(i));
        }
        return result;
    }

    Coordinates max(Coordinates const& other) const {
        Coordinates result;
        for (std::size_t i = 0; i < num_dimensions; ++i) {
            result.at(i) = std::max(indices.at(i), other.at(i));
        }
        return result;
    }
};

#endif
