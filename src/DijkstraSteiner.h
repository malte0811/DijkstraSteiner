#ifndef DIJKSTRA_STEINER
#define DIJKSTRA_STEINER

#include "TypeDefs.h"
#include "LabelMap.h"
#include "HananGrid.h"
#include "future_costs/FutureCost.h"
#include <iostream>
#include <limits>
#include <stdexcept>
#include <utility>
#include <queue>
#include <unordered_map>
#include <cassert>

template<typename T>
concept SubsetConsumer = requires(T a, TerminalSubset l) {
    a(l);
};

template<FutureCost FC>
class DijkstraSteiner {
public:
    DijkstraSteiner(HananGrid grid):
        _grid(std::move(grid)),
        _future_cost{_grid},
        _node_states(_grid, std::numeric_limits<Cost>::max(), false)
    {}

    Cost get_optimum_cost();
private:
    struct HeapEntry {
        Cost cost_lower_bound;
        Label label;

        bool operator>(HeapEntry const& other) const {
            return cost_lower_bound > other.cost_lower_bound;
        }
    };

    void init();

    Label get_full_tree_label() const;

    void handle_candidate(Label const& label, Cost const& cost_to_label);

    template<SubsetConsumer Consumer>
    void for_each_disjoint_sink_set(TerminalSubset const& disjoint_to, Consumer out) const;

    bool _started = false;
    MinHeap<HeapEntry> _heap;
    HananGrid const _grid;
    FC _future_cost;
    LabelMap<Cost, bool> _node_states;
};

template<FutureCost FC>
void DijkstraSteiner<FC>::init() {
    assert(not _started);
    _started = true;
    auto const num_non_root_terminals = _grid.get_terminals().size() - 1;
    for (std::size_t terminal_id = 0; terminal_id < num_non_root_terminals; ++terminal_id) {
        TerminalSubset terminals;
        terminals.set(terminal_id);
        handle_candidate({_grid.get_terminals().at(terminal_id), terminals}, 0);
    }
}

template<FutureCost FC>
Label DijkstraSteiner<FC>::get_full_tree_label() const {
    auto const root_terminal = _grid.get_terminals().back();
    auto const num_non_root_terminals = _grid.get_terminals().size() - 1;
    TerminalSubset terminals;
    for (std::size_t terminal_id = 0; terminal_id < num_non_root_terminals; ++terminal_id) {
        terminals.set(terminal_id);
    }
    return Label{root_terminal, terminals};
}

template<FutureCost FC>
Cost DijkstraSteiner<FC>::get_optimum_cost() {
    init();
    auto const stop_at_label = get_full_tree_label();
    while (not _heap.empty()) {
        auto const next_heap_element = _heap.top();
        _heap.pop();
        // Structured binding would be nice here, but that doesn't work nicely with
        // lambda captures
        auto const next_label = next_heap_element.label;
        if (next_label == stop_at_label) {
            // future cost is 0 here
            return next_heap_element.cost_lower_bound;
        }
        auto const node_state  = _node_states.at(next_label.second, next_label.first);
        // ditto
        auto const cost_here = std::get<0>(node_state).get();
        auto is_fixed = std::get<1>(node_state);
        if (is_fixed) {
            continue;
        }
        is_fixed = true;
        _grid.for_each_neighbor(next_label.first, [&](GridPoint neighbor, Cost edge_cost) {
            Label neighbor_label{neighbor, next_label.second};
            handle_candidate(neighbor_label, edge_cost + cost_here);
        });
        for_each_disjoint_sink_set(next_label.second, [&](TerminalSubset const& other_set) {
            assert((other_set & next_label.second).count() == 0);
            auto const [other_cost, other_fixed] = _node_states.at(other_set, next_label.first);
            if (other_fixed) {
                Label union_label{next_label.first, next_label.second | other_set};
                handle_candidate(union_label, other_cost + cost_here);
            }
        });
    }
    std::cerr << "Failed to find a tree, returning cost 0. This should not be possible!\n";
    return 0;
}

template<FutureCost FC>
void DijkstraSteiner<FC>::handle_candidate(Label const& label, Cost const& cost_to_label) {
    auto [cost, fixed] = _node_states.at(label.second, label.first);
    if (not fixed and cost > cost_to_label) {
        cost.get() = cost_to_label;
        _heap.push(HeapEntry{cost_to_label + _future_cost(label), label});
    }
}

template<FutureCost FC>
template<SubsetConsumer Consumer>
void DijkstraSteiner<FC>::for_each_disjoint_sink_set(TerminalSubset const& base_set, Consumer const out) const {
    auto const bitmask = (~base_set) & TerminalSubset{(1ul << (_grid.get_terminals().size() - 1)) - 1ul};
    TerminalSubset current_set;
    do {
        current_set |= base_set;
        auto temp = current_set.to_ulong();
        ++temp;
        current_set = TerminalSubset{temp};
        current_set &= bitmask;
        // Do not call for empty set, as specified in the algorithm
        if (current_set.any()) {
            out(current_set);
        }
    } while (current_set.any());
}

#endif
