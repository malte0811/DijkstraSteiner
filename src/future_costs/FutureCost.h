#ifndef FUTURE_COST_H
#define FUTURE_COST_H

#include "../HananGrid.h"
#include "../SubsetIndexer.h"

template<typename T>
concept FutureCost = requires(T const a, Label l, HananGrid const grid, SubsetIndexer indexer) {
    T{grid, indexer};
    { a(l) } -> convertible_to<Cost>;
};

#endif
