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
};

class AxisGrid {
public:
    AxisGrid(std::vector<Point> const& points, std::size_t dimension);

    AxisGrid() = default;

    TerminalIndex index_for_coord(Coord const pos) const;

    Coord offset;
    std::vector<Coord> differences;
};

template<class T>
concept NeighborVisitor = requires(T a, GridPoint neighbor, Cost cost) {
    a(neighbor, cost);
};

class HananGrid {
public:
    HananGrid(std::vector<Point> const& points);

    template<NeighborVisitor Visitor>
    void for_each_neighbor(GridPoint here, Visitor const& visitor) const;

    auto const& get_terminals() const {
        return _terminals;
    }

    VertexIndex num_vertices() const {
        VertexIndex result = 1;
        for (auto const& axis : _axis_grids) {
            result *= 1 + axis.differences.size();
        }
        return result;
    }

    VertexIndex get_index(GridPoint const& point) const {
        VertexIndex result = 0;
        for (std::size_t i = 0; i < num_dimensions; ++i) {
            result *= _axis_grids.at(i).differences.size() + 1;
            result += point.indices.at(i);
        }
        return result;
    }

private:
    std::array<AxisGrid, num_dimensions> _axis_grids;
    std::vector<GridPoint> _terminals;
};

template<NeighborVisitor Visitor>
void HananGrid::for_each_neighbor(GridPoint const here, Visitor const& visitor) const {
    for (std::size_t dimension = 0; dimension < num_dimensions; ++dimension) {
        auto const& grid = _axis_grids.at(dimension);
        auto const axis_index = here.indices.at(dimension);
        if (axis_index > 0) {
            visitor(here.subtract(dimension, 1), grid.differences.at(axis_index - 1));
        }
        if (axis_index < grid.differences.size()) {
            visitor(here.add(dimension, 1), grid.differences.at(axis_index));
        }
    }
}

#endif
