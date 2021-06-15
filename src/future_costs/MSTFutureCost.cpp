#include "MSTFutureCost.h"
#include <limits>
#include <cassert>

auto constexpr invalid_cost = std::numeric_limits<Cost>::max();

MSTFutureCost::MSTFutureCost(HananGrid const& grid) : _grid(grid) {
    for (TerminalIndex index_a = 0; index_a < grid.num_terminals(); ++index_a) {
        _costs.at(index_a) = grid.get_distances_to_terminals(grid.get_terminals().at(index_a));
    }
}

Cost MSTFutureCost::operator()(Label const& label) const {
    // Compute edges to add to form a 1-tree with label.first as "1"
    if (label.first.global_index != _vertex_for_cached_distances) {
        _cached_distances = _grid.get_distances_to_terminals(label.first);
        _vertex_for_cached_distances = label.first.global_index;
    }
    Cost min_edge = invalid_cost;
    Cost second_min_edge = invalid_cost;
    for_each_set_bit(
        ~label.second, _grid.num_terminals(), [&](auto const set_bit) {
            auto const cost = _cached_distances[set_bit];
            if (cost < second_min_edge) {
                if (cost <= min_edge) {
                    second_min_edge = min_edge;
                    min_edge = cost;
                } else {
                    second_min_edge = cost;
                }
            }
        }
    );
    auto const tree_cost = get_tree_cost(label.second);
    auto const total_one_tree_cost = [&] {
        if (second_min_edge != invalid_cost) {
            return tree_cost + min_edge + second_min_edge;
        } else {
            assert(min_edge != invalid_cost);
            assert(label.second.count() == _grid.num_non_root_terminals());
            return 2 * min_edge;
        }
    }();
    return (total_one_tree_cost + 1) / 2;
}

Cost MSTFutureCost::get_tree_cost(TerminalSubset const& label) const {
    assert(not label.test(_grid.num_terminals() - 1));
    auto const known_it = _known_tree_costs.find(label);
    if (known_it != _known_tree_costs.end()) {
        return known_it->second;
    } else {
        auto const cost = compute_tree_cost(label);
        _known_tree_costs.emplace(label, cost);
        return cost;
    }
}

Cost MSTFutureCost::compute_tree_cost(TerminalSubset const& label) const {
    std::array<TerminalIndex, max_num_terminals> terminals_to_consider;
    std::size_t num_terminals = 0;
    for (TerminalIndex i = 0; i < _grid.num_terminals(); ++i) {
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
    std::array<bool, max_num_terminals> is_connected{};
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
