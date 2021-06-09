#ifndef DIJKSTRA_STEINER
#define DIJKSTRA_STEINER

#include "TypeDefs.h"
#include "LabelMap.h"
#include "HananGrid.h"
#include <bits/c++config.h>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <utility>
#include <queue>
#include <unordered_map>
#include <cassert>

using Label = std::pair<GridPoint, TerminalSubset>;

template<typename T>
concept LowerBound = requires(T a, Label l, HananGrid grid) {
    // TODO pass Hanan grid as well?
    { a(l, grid) } -> convertible_to<Cost>;
};

template<typename T>
concept SubsetConsumer = requires(T a, TerminalSubset l) {
    a(l);
};

template<LowerBound LB>
class DijkstraSteiner {
public:
    DijkstraSteiner(HananGrid grid):
        _grid(std::move(grid)),
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
    void for_each_disjoint_sink_set(Label const& disjoint_to, Consumer out) const;

    bool _started = false;
    MinHeap<HeapEntry> _heap;
    HananGrid const _grid;
    LB _future_cost;
    LabelMap<Cost, bool> _node_states;
};

template<LowerBound LB>
void DijkstraSteiner<LB>::init() {
    assert(not _started);
    _started = true;
    auto const num_non_root_terminals = _grid.get_terminals().size() - 1;
    for (std::size_t terminal_id = 0; terminal_id < num_non_root_terminals; ++terminal_id) {
        TerminalSubset terminals;
        terminals.set(terminal_id);
        handle_candidate({_grid.get_terminals().at(terminal_id), terminals}, 0);
    }
}

template<LowerBound LB>
Label DijkstraSteiner<LB>::get_full_tree_label() const {
    auto const root_terminal = _grid.get_terminals().back();
    auto const num_non_root_terminals = _grid.get_terminals().size() - 1;
    TerminalSubset terminals;
    for (std::size_t terminal_id = 0; terminal_id < num_non_root_terminals; ++terminal_id) {
        terminals.set(terminal_id);
    }
    return Label{root_terminal, terminals};
}

template<LowerBound LB>
Cost DijkstraSteiner<LB>::get_optimum_cost() {
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
        for_each_disjoint_sink_set(next_label, [&](TerminalSubset const& other_set) {
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

template<LowerBound LB>
void DijkstraSteiner<LB>::handle_candidate(Label const& label, Cost const& cost_to_label) {
    auto [cost, fixed] = _node_states.at(label.second, label.first);
    if (cost > cost_to_label) {
        assert(not fixed);
        cost.get() = cost_to_label;
        _heap.push(HeapEntry{cost_to_label + _future_cost(label, _grid), label});
    }
}

template<LowerBound LB>
template<SubsetConsumer Consumer>
void DijkstraSteiner<LB>::for_each_disjoint_sink_set(Label const& base_label, Consumer const out) const {
    std::array<TerminalIndex, max_num_terminals - 1> disjoint_indices;
    auto const total_num_sinks = _grid.get_terminals().size() - 1;
    TerminalIndex next_disjoint_index = 0;
    for (TerminalIndex bit = 0; bit < total_num_sinks; ++bit) {
        if (not base_label.second.test(bit)) {
            disjoint_indices.at(next_disjoint_index) = bit;
            ++next_disjoint_index;
        }
    }
    TerminalSubset disjoint_set;
    bool carry_out;
    do {
        carry_out = true;
        for (TerminalIndex i = 0; carry_out and i < next_disjoint_index; ++i) {
            auto const index = disjoint_indices.at(i);
            carry_out = disjoint_set.test(index);
            disjoint_set.flip(index);
        }
        // Do not call for empty set, as specified in the algorithm
        if (not carry_out) {
            out(disjoint_set);
        }
    } while (not carry_out);
}

#endif
