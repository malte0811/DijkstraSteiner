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
        _best_cost_bounds(_grid, std::numeric_limits<Cost>::max()),
        _is_fixed(_grid, false)
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
    LabelMap<Cost> _best_cost_bounds;
    LabelMap<bool> _is_fixed;
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
        auto const cost_here = _best_cost_bounds.at(next_label);
        auto&& is_fixed = _is_fixed.at(next_label);
        if (is_fixed) {
            continue;
        }
        is_fixed = true;
        _grid.for_each_neighbor(next_label.first, [&](GridPoint neighbor, Cost edge_cost) {
            Label neighbor_label{neighbor, next_label.second};
            handle_candidate(neighbor_label, edge_cost + cost_here);
        });
        for_each_disjoint_sink_set(next_label.second, [&](TerminalSubset const& other_set) {
            assert((other_set & next_label.second).none());
            Label other_label{next_label.first, other_set};
            if (_is_fixed.at(other_label)) {
                auto const other_cost = _best_cost_bounds.at(other_label);
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
    auto& cost_bound = _best_cost_bounds.at(label);
    if (cost_bound > cost_to_label) {
        assert(not _is_fixed.at(label));
        cost_bound = cost_to_label;
        _heap.push(HeapEntry{cost_to_label + _future_cost(label), label});
    }
}

template<FutureCost FC>
template<SubsetConsumer Consumer>
void DijkstraSteiner<FC>::for_each_disjoint_sink_set(TerminalSubset const& base_set, Consumer const out) const {
    // AND-ing with this mask clears the bits we don't want in our disjoint set: Bits over the number of
    // non-root terminals and bits already present in the base_set
    auto const bitmask = (~base_set) & TerminalSubset{(1ul << (_grid.get_terminals().size() - 1)) - 1ul};
    TerminalSubset current_set;
    do {
        current_set |= base_set;
        // Effectively ++current_set
        // C++ does not define an increment operator for bitsets, and we did not consider it appropriate to add
        // one in a header
        // By incrementing the set ORed with the base_set, any carry out of a "block" of zero-bits in base_set
        // is propagated to the next block of zeros.
        {
            auto temp = current_set.to_ulong();
            ++temp;
            current_set = TerminalSubset{temp};
        }
        current_set &= bitmask;
        // Do not call for empty set, as specified in the algorithm
        if (current_set.any()) {
            out(current_set);
        }
    } while (current_set.any());
}

#endif
