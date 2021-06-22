#include "HananGrid.h"
#include <algorithm>
#include <iostream>
#include <istream>
#include <stdexcept>
#include <optional>
#include <cassert>

AxisGrid::AxisGrid(
    std::vector<Point> const& points, std::size_t const dimension, VertexIndex index_factor
) :
    _index_factor(index_factor) {
    _sorted_positions.reserve(points.size());
    for (auto const& point : points) {
        _sorted_positions.push_back(point.at(dimension));
    }

    std::sort(_sorted_positions.begin(), _sorted_positions.end());
    auto const last = std::unique(_sorted_positions.begin(), _sorted_positions.end());
    _sorted_positions.erase(last, _sorted_positions.end());

    _differences.reserve(_sorted_positions.size() - 1);
    for (std::size_t i = 0; i + 1 < _sorted_positions.size(); ++i) {
        _differences.push_back(_sorted_positions.at(i + 1) - _sorted_positions.at(i));
    }
}

TerminalIndex AxisGrid::index_for_coord(Coord const pos) const {
    for (TerminalIndex i = 0; i < _sorted_positions.size(); ++i) {
        if (pos == _sorted_positions.at(i)) {
            return i;
        }
    }
    throw std::runtime_error("Invalid coordinate?");
}

std::optional<Point> read_point(std::istream& in) {
    Point result;
    for (std::size_t i = 0; i < num_dimensions; ++i) {
        in >> result.at(i);
        if (not in) {
            return std::nullopt;
        }
    }
    return result;
}

std::optional<HananGrid> HananGrid::read_from_stream(std::istream& in) {
    // Can't use TerminalIndex=uint8_t=unsigned char here, otherwise C++ will
    // just read the first char and give us that
    std::size_t num_terminals;
    in >> num_terminals;
    if (not in) {
        std::cerr << "Failed to read number of terminals\n";
        return std::nullopt;
    }
    if (num_terminals > max_num_terminals) {
        std::cerr << "Instance specifies " << num_terminals
                  << ", but only " << max_num_terminals << " are supported\n";
        return std::nullopt;
    }
    std::vector<Point> points;
    for (std::size_t terminal = 0; terminal < num_terminals; ++terminal) {
        if (auto const next_point = read_point(in)) {
            points.push_back(next_point.value());
        } else {
            std::cerr << "Failed to read terminal " << terminal << '\n';
            return std::nullopt;
        }
    }
    return HananGrid(points);
}

HananGrid::HananGrid(std::vector<Point> const& terminals) {
    VertexIndex pre_factor = 1;
    for (std::size_t dim = 0; dim < num_dimensions; ++dim) {
        _axis_grids.at(dim) = AxisGrid(terminals, dim, pre_factor);
        pre_factor *= _axis_grids.at(dim).size();
    }
    for (auto const point : terminals) {
        GridPoint::Coordinates coords;
        VertexIndex index = 0;
        for (std::size_t dim = 0; dim < num_dimensions; ++dim) {
            coords.at(dim) = _axis_grids.at(dim).index_for_coord(point.at(dim));
            index += coords.at(dim) * _axis_grids.at(dim).global_index_factor();
        }
        _terminals.push_back({coords, index});
    }
    GridPoint::Coordinates coords{};
    do {
        _vertex_terminal_distances.push_back(compute_distances_to_terminals(coords));
    } while (next(coords));
}

VertexIndex HananGrid::num_vertices() const {
    VertexIndex result = 1;
    for (auto const& axis : _axis_grids) {
        result *= axis.size();
    }
    return result;
}

auto HananGrid::compute_distances_to_terminals(GridPoint::Coordinates from) const -> SingleVertexDistances {
    SingleVertexDistances result;
    auto const center = to_coordinates(from);
    for (TerminalIndex other = 0; other < num_terminals(); ++other) {
        result.at(other) = get_distance(get_terminals().at(other), center);
    }
    return result;
}

Cost HananGrid::get_distance(GridPoint const& grid_point_a, Point const& point_b) const {
    auto const point_a = to_coordinates(grid_point_a.indices);
    Cost terminal_distance = 0;
    for (std::size_t dimension = 0; dimension < num_dimensions; ++dimension) {
        auto const [min, max] = std::minmax(point_a.at(dimension), point_b.at(dimension));
        terminal_distance += max - min;
    }
    return terminal_distance;
}

bool HananGrid::next(GridPoint::Coordinates& in) const {
    for (std::size_t dimension = 0; dimension < num_dimensions; ++dimension) {
        ++in.at(dimension);
        if (in.at(dimension) == _axis_grids.at(dimension).size()) {
            in.at(dimension) = 0;
        } else {
            return true;
        }
    }
    return false;
}
