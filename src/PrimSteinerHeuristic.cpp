#include <cassert>
#include "PrimSteinerHeuristic.h"

PrimSteinerHeuristic::PrimSteinerHeuristic(HananGrid const& grid) :
    _grid(grid),
    _is_vertex_in_tree(grid.num_vertices()) {}

Cost PrimSteinerHeuristic::compute_upper_bound() {
    _is_vertex_in_tree.at(_grid.get_terminals().at(0).global_index) = true;
    for (TerminalIndex i = 1; i < _grid.num_terminals(); ++i) {
        add_terminal_to_tree(i);
    }
    return _total_cost;
}

void PrimSteinerHeuristic::add_terminal_to_tree(TerminalIndex index) {
    std::vector<VertexIndex> predecessors(_grid.num_vertices());
    std::vector<Cost> cost_bounds(_grid.num_vertices(), std::numeric_limits<Cost>::max());
    struct HeapEntry {
        Cost path_length;
        GridPoint vertex_to_fix;

        bool operator>(HeapEntry const& other) const {
            return path_length > other.path_length;
        }
    };
    auto const start = _grid.get_terminals().at(index);
    MinHeap<HeapEntry> heap;
    heap.push({0, start});
    std::optional<VertexIndex> connecting_point;
    while (not connecting_point) {
        assert(not heap.empty());
        auto const next = heap.top();
        auto const& length = next.path_length;
        auto const& vertex = next.vertex_to_fix;
        heap.pop();
        if (length > cost_bounds.at(vertex.global_index)) {
            continue;
        }
        if (_is_vertex_in_tree.at(vertex.global_index)) {
            connecting_point = vertex.global_index;
        } else {
            _grid.for_each_neighbor(
                vertex, [&](GridPoint const& neighbor, Cost cost) {
                    auto const cost_via = length + cost;
                    if (cost_via < cost_bounds.at(neighbor.global_index)) {
                        cost_bounds.at(neighbor.global_index) = cost_via;
                        predecessors.at(neighbor.global_index) = vertex.global_index;
                        heap.push({cost_via, neighbor});
                    }
                }
            );
        }
    }

    _total_cost += cost_bounds.at(connecting_point.value());
    auto path_point = connecting_point.value();
    while (path_point != start.global_index) {
        path_point = predecessors.at(path_point);
        _is_vertex_in_tree.at(path_point) = true;
    }
}
