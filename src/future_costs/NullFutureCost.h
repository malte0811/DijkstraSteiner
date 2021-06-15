#ifndef NULL_FUTURE_COST
#define NULL_FUTURE_COST

#include "FutureCost.h"

struct NullFutureCost {
    explicit NullFutureCost(HananGrid const&) {}

    Cost operator()(Label const&, bool) const {
        return 0;
    }
};

static_assert(FutureCost<NullFutureCost>);

#endif
