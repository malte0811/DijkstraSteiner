#ifndef NULL_FUTURE_COST
#define NULL_FUTURE_COST

#include "FutureCost.h"

struct NullFutureCost {
    NullFutureCost(HananGrid const&) {}

    Cost operator()(Label const&) const {
        return 0;
    }
};

#endif
