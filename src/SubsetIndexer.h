#ifndef SUBSET_INDEXER_H
#define SUBSET_INDEXER_H

#include <unordered_map>
#include "TypeDefs.h"
#include "HananGrid.h"

class SubsetIndexer {
public:
    // TODO add non-inserting version, would fix const-weirdness
    std::size_t get_index_for(TerminalSubset const& subset) const {
        if (subset != _last_query) {
            auto const result_it = _indices.emplace(subset, _indices.size()).first;
            _last_query = subset;
            _last_result = result_it->second;
        }
        return _last_result;
    }
private:
    TerminalSubset mutable _last_query{-1ul};
    std::size_t mutable _last_result;
    std::unordered_map<TerminalSubset, std::size_t> mutable _indices;
};

template<class T>
class SubsetMap {
public:
    SubsetMap(SubsetIndexer& indexer, T initial = T{}): _indexer(indexer), _initial_value(initial) {}

    T& get(TerminalSubset const& subset) {
        auto const index = _indexer.get_index_for(subset);
        if (index >= _storage.size()) {
            _storage.resize(index + 1, _initial_value);
        }
        return _storage[index];
    }

    T const& get(TerminalSubset const& subset) const {
        auto const index = _indexer.get_index_for(subset);
        if (index >= _storage.size()) {
            _storage.resize(index + 1, _initial_value);
        }
        return _storage[index];
    }
private:
    SubsetIndexer& _indexer;
    std::vector<T> mutable _storage;
    T _initial_value;
};

template<class T>
class LabelMap {
public:
    LabelMap(HananGrid const& grid, SubsetIndexer& indexer, T initial):
        _storage(indexer, std::vector<T>(grid.num_vertices(), initial)) {}

    typename std::vector<T>::reference get(Label const& label) {
        return _storage.get(label.second).at(label.first.global_index);
    }

    typename std::vector<T>::const_reference get(Label const& label) const {
        return _storage.get(label.second).at(label.first.global_index);
    }
private:
    SubsetMap<std::vector<T>> _storage;
};

#endif
