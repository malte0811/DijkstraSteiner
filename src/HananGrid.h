#ifndef HANAN_GRID
#define HANAN_GRID

#include "TypeDefs.h"
#include <vector>

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

template<class T>
concept NeighborVisitor = requires(T a, GridPoint neighbor, Cost cost) {
    a(neighbor, cost);
};

class AxisGrid {
public:
    AxisGrid(std::vector<Point> const& points, std::size_t dimension);

    AxisGrid() = default;

    TerminalIndex index_for_coord(Coord pos) const;

    Coord coord_for_index(TerminalIndex index) const;

    std::size_t size() const {
        return sorted_positions.size();
    }

    template<NeighborVisitor Visitor>
    void for_each_neighbor(GridPoint point, std::size_t axis, Visitor const& visitor) const;
private:
    std::vector<Coord> differences;
    std::vector<Coord> sorted_positions;
};

class HananGrid {
public:
    HananGrid(std::vector<Point> const& points);

    template<NeighborVisitor Visitor>
    void for_each_neighbor(GridPoint here, Visitor const& visitor) const;

    auto const& get_terminals() const {
        return _terminals;
    }

    VertexIndex num_vertices() const;

    VertexIndex get_index(GridPoint const& point) const;

    Point to_coordinates(GridPoint const& grid_point) const;
private:
    std::array<AxisGrid, num_dimensions> _axis_grids;
    std::vector<GridPoint> _terminals;
};

template<NeighborVisitor Visitor>
void AxisGrid::for_each_neighbor(GridPoint const here, std::size_t axis, Visitor const& visitor) const {
    auto const axis_index = here.indices.at(axis);
    if (axis_index > 0) {
        visitor(here.subtract(axis, 1), differences.at(axis_index - 1));
    }
    if (axis_index < differences.size()) {
        visitor(here.add(axis, 1), differences.at(axis_index));
    }
}

template<NeighborVisitor Visitor>
void HananGrid::for_each_neighbor(GridPoint const here, Visitor const& visitor) const {
    for (std::size_t dimension = 0; dimension < num_dimensions; ++dimension) {
        _axis_grids.at(dimension).for_each_neighbor(here, dimension, visitor);
    }
}

#endif
