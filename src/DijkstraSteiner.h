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

template<typename T>
concept SubsetConsumer = requires(T a, TerminalSubset l, Cost c) {
    a(l, c);
};

template<FutureCost FC>
class DijkstraSteiner {
public:
    explicit DijkstraSteiner(HananGrid grid) :
        _grid(std::move(grid)),
        _future_cost{_grid, _indexer},
        _fixed_values(_grid.num_vertices()),
        _best_cost_bounds(_grid, _indexer, invalid_cost),
        _fixed(_grid, _indexer, false),
        _lemma_15_subsets(_indexer, TerminalSubset{0}),
        // By using half the maximum value we make sure that we can always add two entries of this map without overflow
        _lemma_15_bounds(_indexer, invalid_cost / 2),
        _cheapest_edge_to_complement(_indexer) {}

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
        Cost distance = invalid_cost;
        TerminalIndex terminal = 0;
    };

    void init();

    /// Computes the label corresponding to the Steiner tree on all terminals
    [[nodiscard]] Label get_full_tree_label() const;

    /**
     * Adds the given candidate to the heap unless the label is already fixed, or we can prove that the candidate can
     * not contribute to an optimum tree.
     */
    void handle_candidate(Label const& label, Cost const& cost_to_label);

    /**
     * Calls the consumer with each subset disjoint to base_label.second for which the label with the same special
     * vertex has already been fixed, and the cost of this label
     */
    template<SubsetConsumer Consumer>
    void for_each_disjoint_fixed_sink_set(Label const& base_label, Consumer out) const;

    /// Updates the data used to prune nodes based on Lemma 15 when the cost of the given label is fixed.
    void update_lemma_15_data_for(Label const& label, Cost label_cost);

    [[nodiscard]] DistanceToTerminal get_closest_terminal_in_complement(TerminalSubset const& terminals) const;

    MinHeap<HeapEntry> _heap;
    HananGrid const _grid;
    /// The indexer used for all Subset- and LabelMaps
    SubsetIndexer _indexer;
    FC _future_cost;
    /// For each vertex v stores all subsets and costs I and c such that (v, I) is a fixed label with cost c
    std::vector<std::vector<std::pair<TerminalSubset, Cost>>> _fixed_values;
    /// l(v, I) at the current point of the algorithm
    LabelMap<Cost> _best_cost_bounds;
    LabelMap<bool> _fixed;
    /// The best known set S (as described in Lemma 15) for each terminal subset I
    SubsetMap<TerminalSubset> _lemma_15_subsets;
    /// c(H) for the subgraphs corresponding to the _lemma_15_subsets
    SubsetMap<Cost> _lemma_15_bounds;
    /// Stores the cost of a cheapest path from a terminal in the given set to one in the complement
    SubsetMap<DistanceToTerminal> mutable _cheapest_edge_to_complement;
    /// Global upper bound on the cost of a Steiner tree
    Cost _upper_cost_bound = 0;
};

template<FutureCost FC>
void DijkstraSteiner<FC>::init() {
    _upper_cost_bound = PrimSteinerHeuristic{_grid}.compute_upper_bound();
    for (std::size_t terminal_id = 0; terminal_id < _grid.num_non_root_terminals(); ++terminal_id) {
        TerminalSubset terminals;
        terminals.set(terminal_id);
        handle_candidate({_grid.get_terminals().at(terminal_id), terminals}, 0);
    }
}

template<FutureCost FC>
Label DijkstraSteiner<FC>::get_full_tree_label() const {
    return Label{_grid.get_terminals().back(), TerminalSubset{(1ul << _grid.num_non_root_terminals()) - 1}};
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
        auto&& is_fixed = _fixed.get_or_insert(next_label, true);
        if (is_fixed) { continue; }
        is_fixed = true;
        auto const cost_here = _best_cost_bounds.get_or_default(next_label);
        if (cost_here > _lemma_15_bounds.get_or_default(next_label.second)) { continue; }
        update_lemma_15_data_for(next_label, cost_here);

        _fixed_values.at(next_label.first.global_index).push_back({next_label.second, cost_here});
        _grid.for_each_neighbor(
            next_label.first, [&](GridPoint neighbor, Cost edge_cost) {
                Label neighbor_label{neighbor, next_label.second};
                handle_candidate(neighbor_label, edge_cost + cost_here);
            }
        );
        // Get this after running update_lemma_15_data_for, since that may improve the bound
        auto const lemma_15_bound = _lemma_15_bounds.get_or_default(next_label.second);
        auto const lemma_15_set = _lemma_15_subsets.get_or_default(next_label.second);
        for_each_disjoint_fixed_sink_set(
            next_label, [&](TerminalSubset const& other_set, Cost const other_cost) {
                assert((other_set & next_label.second).none());
                // Second type of update for the bounds used in Lemma 15, as described in Section 5
                auto const other_lemma15_bound = _lemma_15_bounds.get_or_default(other_set, true);
                auto const& other_label_15_set = _lemma_15_subsets.get_or_default(other_set);
                auto const union_set = next_label.second | other_set;
                auto& union_cost = _lemma_15_bounds.get_or_insert(union_set, true);
                if (lemma_15_bound + other_lemma15_bound < union_cost and
                    ((lemma_15_set & other_set).none() or (other_label_15_set & next_label.second).none())) {
                    _lemma_15_subsets.get_or_insert(union_set) = (lemma_15_set | other_label_15_set) & ~union_set;
                    union_cost = lemma_15_bound + other_lemma15_bound;
                }

                Label union_label{next_label.first, union_set};
                handle_candidate(union_label, other_cost + cost_here);
            }
        );
    }
    std::cerr << "Failed to find a tree, returning cost 0. This should not be possible!\n";
    return 0;
}

template<FutureCost FC>
void DijkstraSteiner<FC>::handle_candidate(Label const& label, Cost const& cost_to_label) {
    // Do not add if already above the global bound without considering future costs
    if (cost_to_label > _upper_cost_bound) { return; }
    if (cost_to_label > _lemma_15_bounds.get_or_default(label.second, true)) { return; }
    auto& cost_bound = _best_cost_bounds.get_or_insert(label);
    if (cost_to_label < cost_bound) {
        assert(not _fixed.get_or_default(label));
        cost_bound = cost_to_label;
        auto const with_future_cost = cost_to_label + _future_cost(label);
        if (with_future_cost > _upper_cost_bound) { return; }
        _heap.push(HeapEntry{with_future_cost, label});
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
    // Use the set that is faster to iterate over in practice. The threshold is purely experimental.
    if (10 * num_subsets <= num_fixed_sets) {
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
            Label other_label{base_label.first, current_set};
            // Do not call for empty set, as specified in the algorithm
            if (current_set.any() and _fixed.get_or_default(other_label, true)) {
                out(current_set, _best_cost_bounds.get_or_default(other_label));
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
    // Try to improve bound by replacing it with a single-vertex bound
    DistanceToTerminal cheapest = get_closest_terminal_in_complement(label.second);
    auto const& distances = _grid.get_distances_to_terminals(label.first.global_index);
    for_each_set_bit(
        ~label.second, _grid.num_terminals(), [&](TerminalIndex not_contained) {
            auto const distance = distances.at(not_contained);
            if (distance < cheapest.distance) {
                cheapest.distance = distance;
                cheapest.terminal = not_contained;
            }
        }
    );
    auto const new_bound = cheapest.distance + label_cost;
    auto& lemma_bound = _lemma_15_bounds.get_or_insert(label.second, true);
    if (new_bound < lemma_bound) {
        lemma_bound = new_bound;
        _lemma_15_subsets.get_or_insert(label.second) = TerminalSubset{1ul << cheapest.terminal};
    }
}

template<FutureCost FC>
auto DijkstraSteiner<FC>::get_closest_terminal_in_complement(
    TerminalSubset const& terminals
) const -> DistanceToTerminal {
    auto& cheapest_edge_from_terminal_set = _cheapest_edge_to_complement.get_or_insert(terminals);
    if (cheapest_edge_from_terminal_set.distance != invalid_cost) {
        return cheapest_edge_from_terminal_set;
    }
    for_each_set_bit(
        terminals, _grid.num_terminals(), [&](TerminalIndex contained) {
            auto const contained_index = _grid.get_terminals().at(contained).global_index;
            auto const& distances = _grid.get_distances_to_terminals(contained_index);
            for_each_set_bit(
                ~terminals, _grid.num_terminals(), [&](TerminalIndex not_contained) {
                    auto const distance = distances.at(not_contained);
                    if (distance < cheapest_edge_from_terminal_set.distance) {
                        cheapest_edge_from_terminal_set.distance = distance;
                        cheapest_edge_from_terminal_set.terminal = not_contained;
                    }
                }
            );
        }
    );
    return cheapest_edge_from_terminal_set;
}

#endif
