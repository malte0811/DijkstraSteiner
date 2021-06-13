#ifndef BB_FUTURECOST_H
#define BB_FUTURECOST_H

#include "FutureCost.h"

struct BBFutureCost {
    HananGrid const& grid;

    Cost operator()(Label const& label) const {
        auto grid_min = label.first.indices;
        auto grid_max = label.first.indices;
        auto const& terminals = grid.get_terminals();
        for_each_set_bit(~label.second, terminals.size(), [&](auto const terminal) {
            grid_min = terminals[terminal].min(grid_min);
            grid_max = terminals[terminal].max(grid_max);
        });
        auto const min_coords = grid.to_coordinates(grid_min);
        auto const max_coords = grid.to_coordinates(grid_max);
        Cost result = 0;
        for (std::size_t dim = 0; dim < num_dimensions; ++dim) {
            result += max_coords.at(dim) - min_coords.at(dim);
        }
        return result;
    }
};

#endif
