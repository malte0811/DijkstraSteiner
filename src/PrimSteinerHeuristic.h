#ifndef PRIMSTEINERHEURISTIC_H
#define PRIMSTEINERHEURISTIC_H

#include "HananGrid.h"

class PrimSteinerHeuristic {
public:
    explicit PrimSteinerHeuristic(HananGrid const& grid);

    Cost compute_upper_bound();

private:
    void add_terminal_to_tree(TerminalIndex index);

    HananGrid const& _grid;

    std::vector<bool> _is_vertex_in_tree;
    Cost _total_cost = 0;
};

#endif
