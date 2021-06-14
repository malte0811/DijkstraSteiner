#ifndef LABEL_MAP_H
#define LABEL_MAP_H

#include <cassert>
#include <iostream>
#include <tuple>
#include <vector>
#include <functional>
#include "TypeDefs.h"
#include "HananGrid.h"
#include "future_costs/FutureCost.h"

template<class T, bool track_set>
class LabelMap {
public:
    using Storage = std::vector<T>;
    using const_reference = typename Storage::const_reference;

    LabelMap(HananGrid const& grid, T initial) :
        _num_non_root_terminals(grid.get_terminals().size() - 1),
        _content(grid.num_vertices() << _num_non_root_terminals, initial),
        _write_accessed_labels(grid.num_vertices()),
        _initial(initial)
    {}

    const_reference get(Label const& label) const {
        // Using vector::at here would increase runtime by ~33%
        return _content[index_for(label)];
    }

    void set(Label const& label, T const new_value) {
        assert(new_value != _initial);
        auto const index = index_for(label);
        auto&& current_value = _content[index];
        if (track_set and current_value == _initial) {
            _write_accessed_labels[label.first.global_index].push_back(label.second);
        }
        current_value = new_value;
    }

    std::size_t num_modified(GridPoint const& v) const {
        static_assert(track_set);
        return _write_accessed_labels[v.global_index].size();
    }

    TerminalSubset const& get_modified(GridPoint const& v, std::size_t const i) const {
        static_assert(track_set);
        return _write_accessed_labels[v.global_index][i];
    }
private:
    std::size_t index_for(Label const& label) const {
        return label.second.to_ulong() | (label.first.global_index << _num_non_root_terminals);
    }

    TerminalIndex _num_non_root_terminals;
    std::vector<T> _content;
    std::vector<std::vector<TerminalSubset>> _write_accessed_labels;
    T _initial;
};

#endif
