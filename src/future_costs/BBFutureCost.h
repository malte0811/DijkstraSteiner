#ifndef BB_FUTURECOST_H
#define BB_FUTURECOST_H

#include "FutureCost.h"

class BBFutureCost {
public:
    BBFutureCost(HananGrid const& grid, SubsetIndexer&): _grid(grid) {}
    Cost operator()(Label const& label) const;
private:
    HananGrid const& _grid;
};

static_assert(FutureCost<BBFutureCost>);

#endif
