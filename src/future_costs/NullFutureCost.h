#ifndef NULL_FUTURE_COST
#define NULL_FUTURE_COST

#include "FutureCost.h"

struct NullFutureCost {
    NullFutureCost(HananGrid const&, SubsetIndexer&) {}

    Cost operator()(Label const&, bool) const {
        return 0;
    }
};

static_assert(FutureCost<NullFutureCost>);

#endif
