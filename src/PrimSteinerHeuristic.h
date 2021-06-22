#ifndef PRIMSTEINERHEURISTIC_H
#define PRIMSTEINERHEURISTIC_H

#include "HananGrid.h"
#include <utility>

class PrimSteinerHeuristic {
public:
    explicit PrimSteinerHeuristic(HananGrid const& grid);

    Cost compute_upper_bound();

private:
    using GridEdge = std::pair<Point, Point>;

    /**
     * Connected the terminal to the tree by a shortest path SP(edge_to_attach_to),
     * splitting the edge as required
     */
    void add_terminal_to_tree(TerminalIndex index, std::size_t edge_attached_to);

    /**
     * Computes a terminal currently not contained in the tree and an edge ID
     * such that distance_to_sp(terminal, edge) is minimum
     */
    std::pair<TerminalIndex, std::size_t> get_closest_terminal_and_edge() const;

    /**
     * Computes argmin{dist(p, v) | v \in SP(edge)}
     */
    static Point get_closest_sp_point(Point const& p, GridEdge const& edge);

    /**
     * Computes min{dist(p, v) | v \in SP(edge)}
     */
    static Cost distance_to_sp(Point const& p, GridEdge const& edge);

    static Cost length(GridEdge const& edge);

    std::vector<bool> _is_terminal_in_tree;
    std::vector<GridEdge> _tree_edges;
    std::vector<Point> _terminals;
};

#endif
