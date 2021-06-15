#ifndef BB_FUTURECOST_H
#define BB_FUTURECOST_H

#include "FutureCost.h"

struct BBFutureCost {
    HananGrid const& grid;

    Cost operator()(Label const& label) const;
};

static_assert(FutureCost<BBFutureCost>);

#endif
