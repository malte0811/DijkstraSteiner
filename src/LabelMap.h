#ifndef LABEL_MAP_H
#define LABEL_MAP_H

#include <iostream>
#include <tuple>
#include <vector>
#include <functional>
#include "TypeDefs.h"
#include "HananGrid.h"

template<class... Ts>
class LabelMap {
    template <typename T>
    static std::reference_wrapper<T> convert (T & t) { return t; }

    template <typename T>
    static T convert (T && t) { return std::move(t); }
public:
    using StoredTypeRefs = std::tuple<decltype(convert(std::declval<typename std::vector<Ts>::reference>()))...>;
    using StoredValues = std::tuple<Ts...>;

    LabelMap(HananGrid const& grid, Ts... initial);

    StoredValues at(TerminalSubset const& subset, GridPoint vertex) const;
    StoredTypeRefs at(TerminalSubset const& subset, GridPoint vertex);
private:
    std::size_t index_for(TerminalSubset const& subset, GridPoint vertex) const;

    TerminalIndex _num_non_root_terminals;
    std::tuple<std::vector<Ts>...> _content;
    HananGrid const& _grid;
};

template<class... Ts>
LabelMap<Ts...>::LabelMap(HananGrid const& grid, Ts... initial)
    : _num_non_root_terminals(grid.get_terminals().size() - 1),
    _content(std::vector<Ts>(grid.num_vertices() << _num_non_root_terminals, initial)...),
    _grid(grid) {}

template<class... Ts>
auto LabelMap<Ts...>::at(TerminalSubset const& subset, GridPoint vertex) const -> StoredValues {
    auto const index = index_for(subset, vertex);
    //TODO use index-based get?
    return std::make_tuple(std::get<std::vector<Ts>>(_content).at(index)...);
}

template<class... Ts>
auto LabelMap<Ts...>::at(TerminalSubset const& subset, GridPoint vertex) -> StoredTypeRefs {
    auto const index = index_for(subset, vertex);
    //TODO use index-based get?
    return std::make_tuple(convert(std::get<std::vector<Ts>>(_content).at(index))...);
}

template<class... Ts>
std::size_t LabelMap<Ts...>::index_for(TerminalSubset const& subset, GridPoint vertex) const {
    auto const result = subset.to_ulong() | (_grid.get_index(vertex) << _num_non_root_terminals);
    return result;
}

#endif
