#include "BBFutureCost.h"

Cost BBFutureCost::operator()(Label const& label) const {
    auto grid_min = label.first.indices;
    auto grid_max = label.first.indices;
    auto const& terminals = _grid.get_terminals();
    for_each_set_bit(
        ~label.second, terminals.size(), [&](auto const terminal) {
            grid_min = terminals[terminal].min(grid_min);
            grid_max = terminals[terminal].max(grid_max);
        }
    );
    return HananGrid::get_distance(_grid.to_coordinates(grid_min), _grid.to_coordinates(grid_max));
}
