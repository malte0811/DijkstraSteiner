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

template<class T>
class LabelMap {
public:
    using Storage = std::vector<T>;
    using const_reference = typename Storage::const_reference;

    LabelMap(HananGrid const& grid, T initial) :
        _num_non_root_terminals(grid.get_terminals().size() - 1),
        _content(grid.num_vertices() << _num_non_root_terminals, initial) {}

    [[nodiscard]] const_reference get(Label const& label) const {
        // Using vector::at here would increase runtime by ~33%
        return _content[index_for(label)];
    }

    void set(Label const& label, T const new_value) {
        _content[index_for(label)] = new_value;
    }

private:
    [[nodiscard]] std::size_t index_for(Label const& label) const {
        return label.second.to_ulong() | (label.first.global_index << _num_non_root_terminals);
    }

    TerminalIndex _num_non_root_terminals;
    std::vector<T> _content;
};

#endif
