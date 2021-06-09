#ifndef HANAN_GRID
#define HANAN_GRID

#include "TypeDefs.h"
#include "GridPoint.h"
#include <vector>
#include <optional>

template<class V>
concept NeighborVisitor = requires(V v, GridPoint neighbor, Cost cost) {
    v(neighbor, cost);
};

/**
 * Stores a single axis of the Hana grid. This is not intended for standalone use,
 * but as a part of HananGrid
 */
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
    static std::optional<HananGrid> read_from_stream(std::istream& in);

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

inline VertexIndex HananGrid::get_index(GridPoint const& point) const {
    VertexIndex result = 0;
    for (std::size_t i = 0; i < num_dimensions; ++i) {
        result *= _axis_grids.at(i).size();
        result += point.indices.at(i);
    }
    return result;
}

inline Point HananGrid::to_coordinates(GridPoint const& grid_point) const {
    Point result;
    for (std::size_t axis = 0; axis < num_dimensions; ++axis) {
        result.at(axis) = _axis_grids.at(axis).coord_for_index(grid_point.indices.at(axis));
    }
    return result;
}

inline Coord AxisGrid::coord_for_index(TerminalIndex index) const {
    return sorted_positions.at(index);
}

#endif
