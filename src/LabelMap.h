#ifndef LABEL_MAP_H
#define LABEL_MAP_H

#include <iostream>
#include <tuple>
#include <vector>
#include <functional>
#include "TypeDefs.h"
#include "HananGrid.h"

/**
 * Maps a label in the Dijkstra-Steiner algorithm to objects of types Ts. Each type is stored
 * in its own vector to minimize memory usage. In particular bool uses std::vector<bool>, with
 * the usual implications.
 */
template<class... Ts>
class LabelMap {
    // Necessary to support bool vectors for the non-const "at"
    template <typename T>
    static std::reference_wrapper<T> convert (T& t) { return t; }

    template <typename T>
    static T convert (T&& t) { return std::move(t); }
public:
    using StoredTypeRefs = std::tuple<decltype(
            convert(std::declval<typename std::vector<Ts>::reference>())
    )...>;
    using StoredValues = std::tuple<Ts...>;

    LabelMap(HananGrid const& grid, Ts... initial);

    StoredValues at(TerminalSubset const& subset, GridPoint const& vertex) const;
    StoredTypeRefs at(TerminalSubset const& subset, GridPoint const& vertex);
private:
    std::size_t index_for(TerminalSubset const& subset, GridPoint const& vertex) const;

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
auto LabelMap<Ts...>::at(
        TerminalSubset const& subset, GridPoint const& vertex
) const -> StoredValues {
    auto const index = index_for(subset, vertex);
    return std::make_tuple(
            std::get<std::vector<Ts>>(_content).at(index)...
    );
}

template<class... Ts>
auto LabelMap<Ts...>::at(
        TerminalSubset const& subset, GridPoint const& vertex
) -> StoredTypeRefs {
    auto const index = index_for(subset, vertex);
    return std::make_tuple(
            convert(std::get<std::vector<Ts>>(_content).at(index))...
    );
}

template<class... Ts>
std::size_t LabelMap<Ts...>::index_for(
        TerminalSubset const& subset, GridPoint const& vertex
) const {
    auto const result = subset.to_ulong() | (vertex.global_index << _num_non_root_terminals);
    return result;
}

#endif
