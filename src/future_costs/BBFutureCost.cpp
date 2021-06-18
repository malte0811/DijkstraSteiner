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
    auto const min_coords = _grid.to_coordinates(grid_min);
    auto const max_coords = _grid.to_coordinates(grid_max);
    Cost result = 0;
    for (std::size_t dim = 0; dim < num_dimensions; ++dim) {
        result += max_coords.at(dim) - min_coords.at(dim);
    }
    return result;
}
