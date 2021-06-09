#ifndef BB_FUTURECOST_H
#define BB_FUTURECOST_H

#include "../DijkstraSteiner.h"

struct BBFutureCost {
    Cost operator()(Label const& label, HananGrid const& grid) {
        auto grid_min = label.first;
        auto grid_max = label.first;
        auto const& terminals = grid.get_terminals();
        for (TerminalIndex terminal = 0; terminal < terminals.size(); ++terminal) {
            if (not label.second.test(terminal)) {
                grid_min = grid_min.min(terminals.at(terminal));
                grid_max = grid_max.max(terminals.at(terminal));
            }
        }
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
