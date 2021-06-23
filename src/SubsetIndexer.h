#ifndef SUBSET_INDEXER_H
#define SUBSET_INDEXER_H

#include <unordered_map>
#include <optional>
#include <cassert>
#include "TypeDefs.h"
#include "HananGrid.h"

class SubsetIndexer {
public:
    std::optional<std::size_t> get_index_for(TerminalSubset const& subset, [[maybe_unused]] bool allow_mismatch) const {
        if (subset != _last_query) {
            assert(allow_mismatch);
            auto const result_it = _indices.find(subset);
            _last_query = subset;
            if (result_it == _indices.end()) {
                _last_result = std::nullopt;
            } else {
                _last_result = result_it->second;
            }
        }
        return _last_result;
    }

    std::size_t get_index_or_insert(TerminalSubset const& subset, [[maybe_unused]] bool allow_mismatch) {
        if (subset != _last_query or not _last_result.has_value()) {
            assert(allow_mismatch or subset == _last_query);
            auto const result_it = _indices.emplace(subset, _indices.size()).first;
            _last_query = subset;
            _last_result = result_it->second;
        }
        return _last_result.value();
    }
private:
    TerminalSubset mutable _last_query{-1ul};
    std::optional<std::size_t> mutable _last_result;
    std::unordered_map<TerminalSubset, std::size_t> _indices;
};

template<class T>
class SubsetMap {
public:
    SubsetMap(SubsetIndexer& indexer, T initial = T{}): _indexer(indexer), _initial_value(initial) {}

    T& get_or_insert(TerminalSubset const& subset, bool allow_mismatch = false) {
        auto const index = _indexer.get_index_or_insert(subset, allow_mismatch);
        if (index >= _storage.size()) {
            _storage.resize(index + 1, _initial_value);
        }
        return _storage[index];
    }

    T const& get_or_default(TerminalSubset const& subset, bool allow_mismatch = false) const {
        auto const index = _indexer.get_index_for(subset, allow_mismatch);
        if (not index.has_value() or index.value() >= _storage.size()) {
            return _initial_value;
        } else {
            return _storage[index.value()];
        }
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
        _storage(indexer, std::vector<T>(grid.num_vertices(), initial)),
        _default{initial} {}

    typename std::vector<T>::reference get_or_insert(Label const& label, bool allow_mismatch = false) {
        return _storage.get_or_insert(label.second, allow_mismatch).at(label.first.global_index);
    }

    typename std::vector<T>::const_reference get_or_default(Label const& label, bool allow_mismatch = false) const {
        return _storage.get_or_default(label.second, allow_mismatch).at(label.first.global_index);
    }
private:
    SubsetMap<std::vector<T>> _storage;
    T _default;
};

#endif
