#include "MSTFutureCost.h"
#include <limits>
#include <cassert>

auto constexpr invalid_cost = std::numeric_limits<Cost>::max();

MSTFutureCost::MSTFutureCost(HananGrid const& grid):
    _grid(grid),
    _known_tree_costs(1 << (grid.get_terminals().size() - 1), invalid_cost) {
    auto const num_terminals = grid.get_terminals().size();
    for (TerminalIndex index_a = 0; index_a < num_terminals; ++index_a) {
        _costs.at(index_a) = get_distances_to_terminals(grid.get_terminals().at(index_a));
    }
}

Cost MSTFutureCost::operator()(Label const& label) const {
    // Compute edges to add to form a 1-tree with label.first as "1"
    if (label.first.global_index != _vertex_for_cached_distances) {
        _cached_distances = get_distances_to_terminals(label.first);
        _vertex_for_cached_distances = label.first.global_index;
    }
    Cost min_edge = invalid_cost;
    Cost second_min_edge = invalid_cost;
    for_each_set_bit(~label.second, _grid.get_terminals().size(), [&](auto const set_bit) {
        auto const cost = _cached_distances[set_bit];
        if (cost < second_min_edge) {
            if (cost <= min_edge) {
                second_min_edge = min_edge;
                min_edge = cost;
            } else {
                second_min_edge = cost;
            }
        }
    });
    auto const tree_cost = get_tree_cost(label.second);
    auto const total_one_tree_cost = [&] {
        if (second_min_edge != invalid_cost) {
            return tree_cost + min_edge + second_min_edge;
        } else {
            assert(min_edge != invalid_cost);
            assert(label.second.count() == _grid.get_terminals().size() - 1);
            return 2 * min_edge;
        }
    }();
    return (total_one_tree_cost + 1) / 2;
}

Cost MSTFutureCost::get_tree_cost(TerminalSubset const& label) const {
    assert(not label.test(_grid.get_terminals().size() - 1));
    if (_known_tree_costs.at(label.to_ulong()) != invalid_cost) {
        return _known_tree_costs.at(label.to_ulong());
    } else {
        return _known_tree_costs.at(label.to_ulong()) = compute_tree_cost(label);
    }
}

Cost MSTFutureCost::compute_tree_cost(TerminalSubset const& label) const {
    std::array<TerminalIndex, max_num_terminals> terminals_to_consider;
    std::size_t num_terminals = 0;
    for (TerminalIndex i = 0; i < _grid.get_terminals().size(); ++i) {
        if (not label.test(i)) {
            terminals_to_consider.at(num_terminals) = i;
            ++num_terminals;
        }
    }
    struct HeapEntry {
        Cost edge_cost;
        TerminalIndex connecting_terminal;

        bool operator>(HeapEntry const& other) const {
            return edge_cost > other.edge_cost;
        }
    };
    MinHeap<HeapEntry> heap;
    heap.push({0, terminals_to_consider.front()});
    std::array<bool, max_num_terminals> is_connected;
    std::fill(is_connected.begin(), is_connected.end(), false);
    std::size_t num_connected = 0;
    Cost total_cost = 0;
    while (num_connected < num_terminals) {
        assert(not heap.empty());
        auto const[cost, new_terminal] = heap.top();
        heap.pop();
        if (not is_connected.at(new_terminal)) {
            is_connected.at(new_terminal) = true;
            total_cost += cost;
            ++num_connected;
            for (std::size_t i = 0; i < num_terminals; ++i) {
                auto const other_terminal_id = terminals_to_consider.at(i);
                if (not is_connected.at(other_terminal_id)) {
                    heap.push({_costs.at(new_terminal).at(other_terminal_id), other_terminal_id});
                }
            }
        }
    }
    return total_cost;
}

auto MSTFutureCost::get_distances_to_terminals(GridPoint from) const -> SingleVertexDistances {
    SingleVertexDistances result;
    auto const center = _grid.to_coordinates(from.indices);
    for (TerminalIndex other = 0; other < _grid.get_terminals().size(); ++other) {
        result.at(other) = get_distance(_grid.get_terminals().at(other), center);
    }
    return result;
}

Cost MSTFutureCost::get_distance(GridPoint const& grid_point_a, Point const& point_b) const {
    auto const point_a = _grid.to_coordinates(grid_point_a.indices);
    Cost terminal_distance = 0;
    for (std::size_t dimension = 0; dimension < num_dimensions; ++dimension) {
        terminal_distance += std::abs(
                static_cast<int>(point_a.at(dimension)) - static_cast<int>(point_b.at(dimension))
        );
    }
    return terminal_distance;
}
