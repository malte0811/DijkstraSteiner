#ifndef SUBSET_INDEXER_H
#define SUBSET_INDEXER_H

#include <unordered_map>
#include <optional>
#include <cassert>
#include "TypeDefs.h"
#include "HananGrid.h"


/**
 * Lazily assigns unique indices to subsets of the terminal set. The last queried subset is cached, so repeated queries
 * for the same subset are fast. On all methods allow_mismatch indicates whether this call is expected to (sometimes)
 * require a new index to be retrieved from the underlying hash map, or whether the call is expected to always hit the
 * cache.
 */
class SubsetIndexer {
public:
    /// Get the index if it has been assigned, or std::nullopt otherwise
    std::optional<std::size_t> get_index_for(TerminalSubset const& subset, bool allow_mismatch) const;

    /// Get the index for the given subset, assigning a new index if none has been assigned yet
    std::size_t get_index_or_insert(TerminalSubset const& subset, bool allow_mismatch);
private:
    TerminalSubset mutable _last_query{-1ul};
    std::optional<std::size_t> mutable _last_result;
    std::unordered_map<TerminalSubset, std::size_t> _indices;
};

/// Lazily maps subsets to values of the specified type.
template<class T>
class SubsetMap {
public:
    SubsetMap(SubsetIndexer& indexer, T initial = T{}): _indexer(indexer), _initial_value(initial) {}

    T& get_or_insert(TerminalSubset const& subset, bool allow_mismatch = false);

    T const& get_or_default(TerminalSubset const& subset, bool allow_mismatch = false) const;
private:
    SubsetIndexer& _indexer;
    std::vector<T> mutable _storage;
    T _initial_value;
};

/**
 * Lazily maps Labels (i.e. terminal subsets with an additional vertex) to values of the specified type. This is
 * implemented as a SubsetMap to vectors of T, which is a slight waste of memory but also fast.
 */
template<class T>
class LabelMap {
public:
    LabelMap(HananGrid const& grid, SubsetIndexer& indexer, T initial):
        _storage(indexer, std::vector<T>(grid.num_vertices(), initial)) {}


    typename std::vector<T>::reference get_or_insert(Label const& label, bool allow_mismatch = false) {
        return _storage.get_or_insert(label.second, allow_mismatch).at(label.first.global_index);
    }

    typename std::vector<T>::const_reference get_or_default(
            Label const& label, bool allow_mismatch = false
    ) const {
        return _storage.get_or_default(label.second, allow_mismatch).at(label.first.global_index);
    }
private:
    SubsetMap<std::vector<T>> _storage;
};

inline std::optional<std::size_t> SubsetIndexer::get_index_for(
    TerminalSubset const& subset, [[maybe_unused]] bool allow_mismatch
) const {
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

inline std::size_t SubsetIndexer::get_index_or_insert(
    TerminalSubset const& subset, [[maybe_unused]] bool allow_mismatch
) {
    if (subset != _last_query or not _last_result.has_value()) {
        assert(allow_mismatch or subset == _last_query);
        auto const result_it = _indices.emplace(subset, _indices.size()).first;
        _last_query = subset;
        _last_result = result_it->second;
    }
    return _last_result.value();
}

template<class T>
T& SubsetMap<T>::get_or_insert(TerminalSubset const& subset, bool allow_mismatch) {
    auto const index = _indexer.get_index_or_insert(subset, allow_mismatch);
    if (index >= _storage.size()) {
        _storage.resize(index + 1, _initial_value);
    }
    return _storage[index];
}

template<class T>
T const& SubsetMap<T>::get_or_default(TerminalSubset const& subset, bool allow_mismatch) const {
    auto const index = _indexer.get_index_for(subset, allow_mismatch);
    if (not index.has_value() or index.value() >= _storage.size()) {
        return _initial_value;
    } else {
        return _storage[index.value()];
    }
}

#endif
