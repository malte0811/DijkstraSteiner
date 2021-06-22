#include <cassert>
#include <algorithm>
#include <numeric>
#include <cstdlib>
#include <iostream>
#include "PrimSteinerHeuristic.h"

PrimSteinerHeuristic::PrimSteinerHeuristic(HananGrid const& grid) :
    _is_terminal_in_tree(grid.num_terminals()) {
    for (auto const& terminal : grid.get_terminals()) {
        _terminals.push_back(grid.to_coordinates(terminal.indices));
    }
}

Cost PrimSteinerHeuristic::compute_upper_bound() {
    _is_terminal_in_tree.at(0) = true;
    // Add zero-length edge to get rid of special case for first edge
    _tree_edges.emplace_back(_terminals.at(0), _terminals.at(0));
    for (std::size_t i = 1; i < _terminals.size(); ++i) {
        auto const[next_terminal, edge_to_split] = get_closest_terminal_and_edge();
        add_terminal_to_tree(next_terminal, edge_to_split);
        _is_terminal_in_tree.at(next_terminal) = true;
    }
    return std::accumulate(_tree_edges.begin(), _tree_edges.end(), 0, [](Cost a, GridEdge const& b) {
        return a + length(b);
    });
}

std::pair<TerminalIndex, std::size_t> PrimSteinerHeuristic::get_closest_terminal_and_edge() const {
    std::optional<std::pair<TerminalIndex, std::size_t>> best;
    std::optional<Cost> best_cost;
    for (TerminalIndex next = 0; next < _terminals.size(); ++next) {
        if (_is_terminal_in_tree.at(next)) { continue; }
        auto const& terminal = _terminals.at(next);
        for (std::size_t edge_id = 0; edge_id < _tree_edges.size(); ++edge_id) {
            auto const distance = distance_to_sp(terminal, _tree_edges.at(edge_id));
            if (not best_cost or distance < *best_cost) {
                best = std::make_pair(next, edge_id);
                best_cost = distance;
            }
        }
    }
    return best.value();
}

void PrimSteinerHeuristic::add_terminal_to_tree(TerminalIndex terminal_id, std::size_t edge_id) {
    auto& edge = _tree_edges.at(edge_id);
    auto const terminal_point = _terminals.at(terminal_id);
    auto const attached_point = get_closest_sp_point(terminal_point, edge);
    // Split edge is required
    if (edge.first != attached_point and edge.second != attached_point) {
        auto const second = edge.second;
        edge.second = attached_point;
        _tree_edges.emplace_back(attached_point, second);
    }
    // Do this after splitting the edge, otherwise the edge-reference may become invalid
    _tree_edges.emplace_back(terminal_point, attached_point);
}

Point PrimSteinerHeuristic::get_closest_sp_point(Point const& p, GridEdge const& edge) {
    Point result{};
    for (std::size_t dimension = 0; dimension < num_dimensions; ++dimension) {
        auto const[min, max] = std::minmax(edge.first.at(dimension), edge.second.at(dimension));
        result.at(dimension) = std::clamp(p.at(dimension), min, max);
    }
    return result;
}

Cost PrimSteinerHeuristic::distance_to_sp(Point const& p, GridEdge const& edge) {
    return length({p, get_closest_sp_point(p, edge)});
}

Cost PrimSteinerHeuristic::length(GridEdge const& edge) {
    Cost result = 0;
    for (std::size_t dimension = 0; dimension < num_dimensions; ++dimension) {
        auto const[min, max] = std::minmax(
                edge.first.at(dimension), edge.second.at(dimension)
        );
        result += max - min;
    }
    return result;
}
