#ifndef FUTURE_COST_H
#define FUTURE_COST_H

#include "../HananGrid.h"

using Label = std::pair<GridPoint, TerminalSubset>;

template<typename T>
concept FutureCost = requires(T const a, Label l, HananGrid grid) {
    T{grid};
    { a(l) } -> convertible_to<Cost>;
};

#endif
