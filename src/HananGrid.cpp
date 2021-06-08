#include "HananGrid.h"
#include <algorithm>
#include <utility>
#include <tuple>
#include <optional>
#include <cassert>

AxisGrid::AxisGrid(std::vector<Point> const& points, std::size_t const dimension) {
    std::vector<Coord> positions;
    positions.reserve(points.size());
    for (auto const& point : points) {
        positions.push_back(point.at(dimension));
    }
    std::sort(positions.begin(), positions.end());
    auto const last = std::unique(positions.begin(), positions.end());
    positions.erase(last, positions.end());
    differences.reserve(positions.size() - 1);
    for (std::size_t i = 0; i + 1 < positions.size(); ++i) {
        differences.push_back(positions.at(i + 1) - positions.at(i));
    }
    offset = positions.front();
}

TerminalIndex AxisGrid::index_for_coord(Coord const pos) const {
    Coord current = offset;
    for (TerminalIndex i = 0; i < differences.size(); ++i) {
        if (pos == current) {
            return i;
        }
        current += differences.at(i);
    }
    assert(current == pos);
    return differences.size();;
}

HananGrid::HananGrid(std::vector<Point> const& points) {
    for (std::size_t dim = 0; dim < num_dimensions; ++dim) {
        _axis_grids.at(dim) = AxisGrid(points, dim);
    }
    for (auto const point : points) {
        GridPoint grid_point;
        for (std::size_t dim = 0; dim < num_dimensions; ++dim) {
            grid_point.indices.at(dim) = _axis_grids.at(dim).index_for_coord(point.at(dim));
        }
        _terminals.push_back(grid_point);
    }
}
