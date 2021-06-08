#ifndef NULL_FUTURE_COST
#define NULL_FUTURE_COST

#include "../DijkstraSteiner.h"

struct NullFutureCost {
    Cost operator()(Label const& /*label*/) {
        return 0;
    }
};

#endif
