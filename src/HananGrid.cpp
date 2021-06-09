#include "HananGrid.h"
#include <algorithm>
#include <iostream>
#include <istream>
#include <stdexcept>
#include <utility>
#include <tuple>
#include <optional>
#include <cassert>

AxisGrid::AxisGrid(std::vector<Point> const& points, std::size_t const dimension) {
    sorted_positions.reserve(points.size());
    for (auto const& point : points) {
        sorted_positions.push_back(point.at(dimension));
    }

    std::sort(sorted_positions.begin(), sorted_positions.end());
    auto const last = std::unique(sorted_positions.begin(), sorted_positions.end());
    sorted_positions.erase(last, sorted_positions.end());

    differences.reserve(sorted_positions.size() - 1);
    for (std::size_t i = 0; i + 1 < sorted_positions.size(); ++i) {
        differences.push_back(sorted_positions.at(i + 1) - sorted_positions.at(i));
    }
}

TerminalIndex AxisGrid::index_for_coord(Coord const pos) const {
    for (TerminalIndex i = 0; i < sorted_positions.size(); ++i) {
        if (pos == sorted_positions.at(i)) {
            return i;
        }
    }
    throw std::runtime_error("Invalid coordinate?");
}

Coord AxisGrid::coord_for_index(TerminalIndex index) const {
    return sorted_positions.at(index);
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

VertexIndex HananGrid::get_index(GridPoint const& point) const {
    VertexIndex result = 0;
    for (std::size_t i = 0; i < num_dimensions; ++i) {
        result *= _axis_grids.at(i).size();
        result += point.indices.at(i);
    }
    return result;
}

VertexIndex HananGrid::num_vertices() const {
    VertexIndex result = 1;
    for (auto const& axis : _axis_grids) {
        result *= axis.size();
    }
    return result;
}

Point HananGrid::to_coordinates(GridPoint const& grid_point) const {
    Point result;
    for (std::size_t axis = 0; axis < num_dimensions; ++axis) {
        result.at(axis) = _axis_grids.at(axis).coord_for_index(grid_point.indices.at(axis));
    }
    return result;
}
