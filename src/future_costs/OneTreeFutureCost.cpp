#include "OneTreeFutureCost.h"
#include <limits>
#include <cassert>

OneTreeFutureCost::OneTreeFutureCost(HananGrid const& grid, SubsetIndexer& indexer) :
    _grid(grid),
    _known_tree_costs(indexer, invalid_cost) {
    for (TerminalIndex index_a = 0; index_a < grid.num_terminals(); ++index_a) {
        auto const vertex_index = grid.get_terminals().at(index_a).global_index;
        _terminal_distances.at(index_a) = grid.get_distances_to_terminals(vertex_index);
    }
}

Cost OneTreeFutureCost::operator()(Label const& label) const {
    // Find cheapest edges to complete the 1-tree (combined with an MST on ~label.second)
    Cost min_edge = invalid_cost;
    Cost second_min_edge = invalid_cost;
    auto const& distances = _grid.get_distances_to_terminals(label.first.global_index);
    for_each_set_bit(
        ~label.second, _grid.num_terminals(), [&](auto const set_bit) {
            auto const cost = distances[set_bit];
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
    if (second_min_edge != invalid_cost) {
        return (tree_cost + min_edge + second_min_edge + 1) / 2;
    } else {
        assert(min_edge != invalid_cost);
        assert(tree_cost == 0);
        assert(label.second.count() == _grid.num_non_root_terminals());
        return min_edge;
    }
}

Cost OneTreeFutureCost::get_tree_cost(TerminalSubset const& label) const {
    assert(not label.test(_grid.num_terminals() - 1));
    auto& cost = _known_tree_costs.get_or_insert(label);
    if (cost != invalid_cost) {
        return cost;
    }
    // Prims algorithm
    std::vector<TerminalIndex> terminals_to_consider;
    terminals_to_consider.reserve(_grid.num_terminals() - label.count());
    for_each_set_bit(~label, _grid.num_terminals(), [&](auto const set_bit) {
        terminals_to_consider.push_back(set_bit);
    });
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
    while (num_connected < terminals_to_consider.size()) {
        assert(not heap.empty());
        auto const[edge_cost, new_terminal] = heap.top();
        heap.pop();
        if (is_connected.at(new_terminal)) { continue; }
        is_connected.at(new_terminal) = true;
        cost += edge_cost;
        ++num_connected;
        for (auto const other_terminal_id : terminals_to_consider) {
            if (not is_connected.at(other_terminal_id)) {
                heap.push({_terminal_distances.at(new_terminal).at(other_terminal_id), other_terminal_id});
            }
        }
    }
    return cost;
}
