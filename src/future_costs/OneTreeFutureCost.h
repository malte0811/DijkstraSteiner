#ifndef MST_FUTURE_COST
#define MST_FUTURE_COST

#include "FutureCost.h"
#include <limits>
#include <unordered_map>

/**
 * Half of the cost of a 1-tree on the vertices not contained in a label and the special vertex as "1" (rounded up)
 * forms a valid future cost (Lemma 8 in arXiv:1406.0492)
 */
class OneTreeFutureCost {
public:
    explicit OneTreeFutureCost(HananGrid const& grid, SubsetIndexer& indexer);

    Cost operator()(Label const& label) const;

private:
    using SingleVertexDistances = HananGrid::SingleVertexDistances;

    Cost get_tree_cost(TerminalSubset const& not_contained_vertices) const;

    Cost compute_tree_cost(TerminalSubset const& not_contained_vertices) const;

    HananGrid const& _grid;
    /// Stores the distances between all pairs of terminals
    std::array<SingleVertexDistances, max_num_terminals> _terminal_distances{};
    /// Stores the known costs of MSTs on subsets of the terminal set
    SubsetMap<Cost> mutable _known_tree_costs;
};

static_assert(FutureCost<OneTreeFutureCost>);

#endif
