#ifndef GRID_POINT_H
#define GRID_POINT_H

#include <array>
#include "TypeDefs.h"

class HananGrid;

struct GridPoint {
    using Coordinates = std::array<TerminalIndex, num_dimensions>;

    /// Indices of the point in the Hanan grid
    Coordinates indices;
    /// A unique index for this vertex. This is computed as \sum_{k=1}^d a_k indices[k] for some values a_k
    VertexIndex global_index;

    /**
     * Computes the next point in the grid along the given coordinate
      * @param axis_factor a_{coordinate} in the definition of global_index
      */
    GridPoint next(std::size_t coordinate, VertexIndex axis_factor) const;

    /// See next
    GridPoint previous(std::size_t coordinate, VertexIndex axis_factor) const;

    bool operator==(GridPoint const& other) const;

    /// Entry-wise minimum
    Coordinates min(Coordinates const& other) const;

    /// Entry-wise maximum
    Coordinates max(Coordinates const& other) const;
};

inline GridPoint GridPoint::next(std::size_t coordinate, VertexIndex axis_factor) const {
    auto new_indices = indices;
    ++new_indices.at(coordinate);
    return {new_indices, static_cast<VertexIndex>(global_index + axis_factor)};
}

inline GridPoint GridPoint::previous(std::size_t coordinate, VertexIndex axis_factor) const {
    auto new_indices = indices;
    --new_indices.at(coordinate);
    return {new_indices, static_cast<VertexIndex>(global_index - axis_factor)};
}

inline bool GridPoint::operator==(GridPoint const& other) const {
    return global_index == other.global_index;
}

inline GridPoint::Coordinates GridPoint::min(GridPoint::Coordinates const& other) const {
    Coordinates result;
    for (std::size_t i = 0; i < num_dimensions; ++i) {
        result.at(i) = std::min(indices.at(i), other.at(i));
    }
    return result;
}

inline GridPoint::Coordinates GridPoint::max(GridPoint::Coordinates const& other) const {
    Coordinates result;
    for (std::size_t i = 0; i < num_dimensions; ++i) {
        result.at(i) = std::max(indices.at(i), other.at(i));
    }
    return result;
}

#endif
