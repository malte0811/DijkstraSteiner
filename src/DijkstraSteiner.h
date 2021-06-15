#ifndef DIJKSTRA_STEINER
#define DIJKSTRA_STEINER

#include "TypeDefs.h"
#include "HananGrid.h"
#include "future_costs/FutureCost.h"
#include "PrimSteinerHeuristic.h"
#include <iostream>
#include <limits>
#include <stdexcept>
#include <utility>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <cassert>

namespace std {
template<>
struct hash<Label> {
    std::size_t operator()(Label const& label) const {
        return label.first.global_index << max_num_terminals | label.second.to_ulong();
    }
};
}

template<typename T>
concept SubsetConsumer = requires(T a, TerminalSubset l, Cost c) {
    a(l, c);
};

template<FutureCost FC>
class DijkstraSteiner {
public:
    explicit DijkstraSteiner(HananGrid grid) :
        _grid(std::move(grid)),
        _future_cost{_grid},
        _fixed_values(_grid.num_vertices()) {}

    [[nodiscard]] Cost get_optimum_cost();

private:
    struct HeapEntry {
        Cost cost_lower_bound{};
        Label label;

        bool operator>(HeapEntry const& other) const {
            return cost_lower_bound > other.cost_lower_bound;
        }
    };

    struct DistanceToTerminal {
        Cost distance = std::numeric_limits<Cost>::max();
        TerminalIndex terminal = 0;
    };

    void init();

    [[nodiscard]] Label get_full_tree_label() const;

    void handle_candidate(Label const& label, Cost const& cost_to_label);

    template<SubsetConsumer Consumer>
    void for_each_disjoint_fixed_sink_set(Label const& base_label, Consumer out) const;

    void update_lemma_15_data_for(Label const& label, Cost label_cost);

    [[nodiscard]] DistanceToTerminal get_closest_terminal_in_complement(TerminalSubset const& terminals) const;

    bool _started = false;
    MinHeap<HeapEntry> _heap;
    HananGrid const _grid;
    FC _future_cost;
    std::vector<std::vector<std::pair<TerminalSubset, Cost>>> _fixed_values;
    std::unordered_map<Label, Cost> _best_cost_bounds;
    std::unordered_set<Label> _fixed;
    std::unordered_map<TerminalSubset, TerminalSubset> _lemma_15_subsets;
    std::unordered_map<TerminalSubset, Cost> _lemma_15_bounds;
    std::unordered_map<TerminalSubset, DistanceToTerminal> mutable _cheapest_edge_to_complement;
    Cost _upper_cost_bound = 0;
};

template<FutureCost FC>
void DijkstraSteiner<FC>::init() {
    assert(not _started);
    _started = true;
    _upper_cost_bound = PrimSteinerHeuristic{_grid}.compute_upper_bound();
    for (std::size_t terminal_id = 0; terminal_id < _grid.num_non_root_terminals(); ++terminal_id) {
        TerminalSubset terminals;
        terminals.set(terminal_id);
        handle_candidate({_grid.get_terminals().at(terminal_id), terminals}, 0);
    }
}

template<FutureCost FC>
Label DijkstraSteiner<FC>::get_full_tree_label() const {
    return Label{
        _grid.get_terminals().back(),
        TerminalSubset{(1ul << _grid.num_non_root_terminals()) - 1}
    };
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
        if (not _fixed.insert(next_label).second) {
            continue;
        }
        auto const cost_here = _best_cost_bounds.at(next_label);
        auto const lemma_bound_it = _lemma_15_bounds.find(next_label.second);
        if (lemma_bound_it != _lemma_15_bounds.end() and lemma_bound_it->second < cost_here) {
            continue;
        }
        update_lemma_15_data_for(next_label, cost_here);

        _fixed_values.at(next_label.first.global_index).push_back({next_label.second, cost_here});
        _grid.for_each_neighbor(
            next_label.first, [&](GridPoint neighbor, Cost edge_cost) {
                Label neighbor_label{neighbor, next_label.second};
                handle_candidate(neighbor_label, edge_cost + cost_here);
            }
        );
        for_each_disjoint_fixed_sink_set(
            next_label, [&](TerminalSubset const& other_set, Cost const other_cost) {
                assert((other_set & next_label.second).none());
                Label union_label{next_label.first, next_label.second | other_set};
                handle_candidate(union_label, other_cost + cost_here);
            }
        );
    }
    std::cerr << "Failed to find a tree, returning cost 0. This should not be possible!\n";
    return 0;
}

template<FutureCost FC>
void DijkstraSteiner<FC>::handle_candidate(Label const& label, Cost const& cost_to_label) {
    if (cost_to_label > _upper_cost_bound) {
        return;
    }
    auto const specific_upper_bound_it = _lemma_15_bounds.find(label.second);
    if (specific_upper_bound_it != _lemma_15_bounds.end() and cost_to_label > specific_upper_bound_it->second) {
        return;
    }
    auto const cost_bound_it = _best_cost_bounds.find(label);
    if (cost_bound_it == _best_cost_bounds.end() or cost_bound_it->second > cost_to_label) {
        assert(not _fixed.count(label));
        auto const with_fc = cost_to_label + _future_cost(label);
        _best_cost_bounds.insert_or_assign(label, cost_to_label);
        if (with_fc > _upper_cost_bound) {
            return;
        }
        _heap.push(HeapEntry{with_fc, label});
    }
}

template<FutureCost FC>
template<SubsetConsumer Consumer>
void DijkstraSteiner<FC>::for_each_disjoint_fixed_sink_set(Label const& base_label, Consumer const out) const {
    auto const& base_set = base_label.second;
    auto const disjoint_bits = _grid.num_non_root_terminals() - base_set.count();
    auto const num_subsets = (1ul << disjoint_bits) - 1ul;
    auto const& fixed_vector = _fixed_values.at(base_label.first.global_index);
    auto const num_fixed_sets = fixed_vector.size();
    if (num_subsets <= num_fixed_sets) {
        // AND-ing with this mask clears the bits we don't want in our disjoint set: Bits over the number of
        // non-root terminals and bits already present in the base_set
        auto const bitmask = (~base_set) & TerminalSubset{(1ul << _grid.num_non_root_terminals()) - 1ul};
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
            Label other_label{base_label.first, current_set};
            if (current_set.any() and _fixed.count(other_label)) {
                out(current_set, _best_cost_bounds.at(other_label));
            }
        } while (current_set.any());
    } else {
        for (auto const&[subset, cost] : fixed_vector) {
            if ((subset & base_set).none()) {
                out(subset, cost);
            }
        }
    }
}

template<FutureCost FC>
void DijkstraSteiner<FC>::update_lemma_15_data_for(Label const& label, Cost const label_cost) {
    DistanceToTerminal cheapest = get_closest_terminal_in_complement(label.second);
    auto const extra_point = _grid.to_coordinates(label.first.indices);
    for_each_set_bit(
        ~label.second, _grid.num_terminals(), [&](TerminalIndex not_contained) {
            auto const distance = _grid.get_distance(_grid.get_terminals().at(not_contained), extra_point);
            if (distance < cheapest.distance) {
                cheapest.distance = distance;
                cheapest.terminal = not_contained;
            }
        }
    );
    auto const new_bound = cheapest.distance + label_cost;
    auto const lemma_bound_it = _lemma_15_bounds.find(label.second);
    if (lemma_bound_it == _lemma_15_bounds.end() or new_bound < lemma_bound_it->second) {
        _lemma_15_bounds.insert_or_assign(label.second, new_bound);
        _lemma_15_subsets.insert_or_assign(label.second, TerminalSubset{1ul << cheapest.terminal});
    }
}

template<FutureCost FC>
auto DijkstraSteiner<FC>::get_closest_terminal_in_complement(
    TerminalSubset const& terminals
) const -> DistanceToTerminal {
    auto const cheapest_edge_from_terminal_set_it = _cheapest_edge_to_complement.find(terminals);
    if (cheapest_edge_from_terminal_set_it != _cheapest_edge_to_complement.end()) {
        return cheapest_edge_from_terminal_set_it->second;
    }
    DistanceToTerminal cheapest_edge_from_terminal_set;
    for_each_set_bit(
        terminals, _grid.num_terminals(), [&](TerminalIndex contained) {
            auto const contained_point = _grid.to_coordinates(_grid.get_terminals().at(contained).indices);
            for_each_set_bit(
                ~terminals, _grid.num_terminals(), [&](TerminalIndex not_contained) {
                    auto const distance = _grid.get_distance(
                        _grid.get_terminals().at(not_contained), contained_point
                    );
                    if (distance < cheapest_edge_from_terminal_set.distance) {
                        cheapest_edge_from_terminal_set.distance = distance;
                        cheapest_edge_from_terminal_set.terminal = not_contained;
                    }
                }
            );
        }
    );
    _cheapest_edge_to_complement.insert({terminals, cheapest_edge_from_terminal_set});
    return cheapest_edge_from_terminal_set;
}

#endif
