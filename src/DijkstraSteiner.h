#ifndef DIJKSTRA_STEINER
#define DIJKSTRA_STEINER

#include "TypeDefs.h"
#include "LabelMap.h"
#include "HananGrid.h"
#include <bits/c++config.h>
#include <limits>
#include <stdexcept>
#include <utility>
#include <queue>
#include <unordered_map>
#include <cassert>

using Label = std::pair<GridPoint, TerminalSubset>;

template<typename T>
concept LowerBound = requires(T a, Label l) {
    // TODO pass Hanan grid as well?
    { a(l) } -> convertible_to<Cost>;
};

template<typename T>
concept SubsetConsumer = requires(T a, TerminalSubset l) {
    a(l);
};

namespace std {
    template<>
    struct hash<Label> {
        std::size_t operator()(Label const& s) const noexcept {
            std::size_t result = 0;
            for (std::size_t i = 0; i < num_dimensions; ++i) {
                result = result * 31 + s.first.indices.at(i);
            }
            result = result * 31 + std::hash<TerminalSubset>{}(s.second);
            return result;
        }
    };
}

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

    void handle_candidate(Label const& label, Cost const& cost_to_label);

    template<SubsetConsumer Consumer>
    void for_each_fixed_disjoint_sink_set(Label const& disjoint_to, Consumer out) const;

    MinHeap<HeapEntry> _heap;
    HananGrid const _grid;
    LB _future_cost;
    LabelMap<Cost, bool> _node_states;
};

template<LowerBound LB>
Cost DijkstraSteiner<LB>::get_optimum_cost() {
    auto const non_root_terminals = [&] {
        auto temp = _grid.get_terminals();
        temp.pop_back();
        return temp;
    }();
    auto const root_terminal = _grid.get_terminals().back();
    for (std::size_t terminal_id = 0; terminal_id < non_root_terminals.size(); ++terminal_id) {
        TerminalSubset terminals;
        terminals.set(terminal_id);
        handle_candidate({non_root_terminals.at(terminal_id), terminals}, 0);
    }
    auto const stop_at_label = [&] {
        TerminalSubset terminals;
        for (std::size_t terminal_id = 0; terminal_id < non_root_terminals.size(); ++terminal_id) {
            terminals.set(terminal_id);
        }
        return Label{root_terminal, terminals};
    }();
    while (not _heap.empty()) {
        auto const [cost_with_lb, next_label] = _heap.top();
        _heap.pop();
        if (next_label == stop_at_label) {
            // future cost is 0 here
            return cost_with_lb;
        }
        auto[cost, fixed]  = _node_states.at(next_label.second, next_label.first);
        if (fixed) {
            continue;
        }
        fixed = true;
        _grid.for_each_neighbor(next_label.first, [&, next_label = next_label, cost = cost](GridPoint neighbor, Cost edge_cost) {
            Label neighbor_label{neighbor, next_label.second};
            auto const candidate_cost = edge_cost + cost;
            handle_candidate(neighbor_label, candidate_cost);
        });
        for_each_fixed_disjoint_sink_set(next_label, [&, next_label = next_label, cost = cost](TerminalSubset const& other_set) {
            assert((other_set & next_label.second).count() == 0);
            Label union_label{next_label.first, next_label.second | other_set};
            auto const candidate_cost = std::get<0>(_node_states.at(other_set, next_label.first)) + cost;
            handle_candidate(union_label, candidate_cost);
        });
    }
    //TODO should never happen!
    throw std::runtime_error("No tree found???");
}

template<LowerBound LB>
void DijkstraSteiner<LB>::handle_candidate(Label const& label, Cost const& cost_to_label) {
    auto [cost, fixed] = _node_states.at(label.second, label.first);
    if (cost > cost_to_label) {
        assert(not fixed);
        cost.get() = cost_to_label;
        _heap.push(HeapEntry{cost_to_label + _future_cost(label), label});
    }
}

template<LowerBound LB>
template<SubsetConsumer Consumer>
void DijkstraSteiner<LB>::for_each_fixed_disjoint_sink_set(Label const& base_label, Consumer const out) const {
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
        if (not carry_out) {
            Label resulting_label{base_label.first, disjoint_set};
            if (std::get<bool>(_node_states.at(disjoint_set, base_label.first))) {
                out(resulting_label.second);
            }
        }
    } while (not carry_out);
}

#endif
