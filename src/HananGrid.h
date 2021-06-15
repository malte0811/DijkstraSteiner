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
    AxisGrid(
        std::vector<Point> const& points, std::size_t dimension, VertexIndex index_factor
    );

    AxisGrid() = default;

    TerminalIndex index_for_coord(Coord pos) const;

    Coord coord_for_index(TerminalIndex index) const;

    std::size_t size() const {
        return _sorted_positions.size();
    }

    template<NeighborVisitor Visitor>
    void for_each_neighbor(GridPoint point, std::size_t axis, Visitor const& visitor) const;

    VertexIndex global_index_factor() const {
        return _index_factor;
    }
private:
    std::vector<Coord> _differences;
    std::vector<Coord> _sorted_positions;
    VertexIndex _index_factor;
};

class HananGrid {
public:
    using SingleVertexDistances = std::array<Cost, max_num_terminals>;

    static std::optional<HananGrid> read_from_stream(std::istream& in);

    HananGrid(std::vector<Point> const& points);

    template<NeighborVisitor Visitor>
    void for_each_neighbor(GridPoint here, Visitor const& visitor) const;

    auto const& get_terminals() const {
        return _terminals;
    }

    TerminalIndex num_terminals() const {
        return _terminals.size();
    }

    TerminalIndex num_non_root_terminals() const {
       return num_terminals() - 1;
    }

    VertexIndex num_vertices() const;

    Point to_coordinates(GridPoint::Coordinates const& grid_point) const;

    SingleVertexDistances get_distances_to_terminals(GridPoint from) const;

    Cost get_distance(GridPoint const& point_a, Point const& point_b) const;
private:
    std::array<AxisGrid, num_dimensions> _axis_grids;
    std::vector<GridPoint> _terminals;
};

template<NeighborVisitor Visitor>
void AxisGrid::for_each_neighbor(GridPoint const here, std::size_t axis, Visitor const& visitor) const {
    auto const axis_index = here.indices.at(axis);
    if (axis_index > 0) {
        visitor(here.previous(axis, _index_factor), _differences.at(axis_index - 1));
    }
    if (axis_index < _differences.size()) {
        visitor(here.next(axis, _index_factor), _differences.at(axis_index));
    }
}

template<NeighborVisitor Visitor>
void HananGrid::for_each_neighbor(GridPoint const here, Visitor const& visitor) const {
    for (std::size_t dimension = 0; dimension < num_dimensions; ++dimension) {
        _axis_grids.at(dimension).for_each_neighbor(here, dimension, visitor);
    }
}

inline Point HananGrid::to_coordinates(GridPoint::Coordinates const& grid_point) const {
    Point result;
    for (std::size_t axis = 0; axis < num_dimensions; ++axis) {
        result.at(axis) = _axis_grids.at(axis).coord_for_index(grid_point.at(axis));
    }
    return result;
}

inline Coord AxisGrid::coord_for_index(TerminalIndex index) const {
    return _sorted_positions.at(index);
}

#endif
