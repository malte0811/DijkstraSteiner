#ifndef MAX_FUTURE_COST_H
#define MAX_FUTURE_COST_H

#include "FutureCost.h"

template<FutureCost CostA, FutureCost CostB>
class MaxFutureCost {
public:
    explicit MaxFutureCost(HananGrid const& grid, SubsetIndexer& indexer) :
        _cost_a{grid, indexer},
        _cost_b{grid, indexer} {}

    Cost operator()(Label const& label) const {
        return std::max(_cost_a(label), _cost_b(label));
    }

private:
    CostA _cost_a;
    CostB _cost_b;
};

#endif
