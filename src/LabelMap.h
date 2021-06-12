#ifndef LABEL_MAP_H
#define LABEL_MAP_H

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
    using reference = typename Storage::reference;
    using const_reference = typename Storage::const_reference;

    LabelMap(HananGrid const& grid, T initial);

    const_reference at(Label const& label) const;
    reference at(Label const& label);
private:
    std::size_t index_for(Label const& label) const;

    TerminalIndex _num_non_root_terminals;
    std::vector<T> _content;
    HananGrid const& _grid;
};

template<class T>
LabelMap<T>::LabelMap(HananGrid const& grid, T initial)
    : _num_non_root_terminals(grid.get_terminals().size() - 1),
    _content(grid.num_vertices() << _num_non_root_terminals, initial),
    _grid(grid) {}

template<class T>
auto LabelMap<T>::at(Label const& label) const -> const_reference {
    return _content.at(index_for(label));
}

template<class T>
auto LabelMap<T>::at(Label const& label) -> reference {
    return _content.at(index_for(label));
}

template<class T>
std::size_t LabelMap<T>::index_for(Label const& label) const {
    return label.second.to_ulong() | (label.first.global_index << _num_non_root_terminals);
}

#endif
