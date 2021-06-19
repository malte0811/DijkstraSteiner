#ifndef MST_FUTURE_COST
#define MST_FUTURE_COST

#include "FutureCost.h"
#include <limits>
#include <unordered_map>

class MSTFutureCost {
public:
    explicit MSTFutureCost(HananGrid const& grid, SubsetIndexer& indexer);

    Cost operator()(Label const& label) const;

    Cost compute_tree_cost(TerminalSubset const& not_contained_vertices) const;

private:
    using SingleVertexDistances = HananGrid::SingleVertexDistances;

    Cost get_tree_cost(TerminalSubset const& not_contained_vertices) const;

    HananGrid const& _grid;
    std::array<SingleVertexDistances, max_num_terminals> _terminal_distances{};
    SubsetMap<Cost> mutable _known_tree_costs;
};

static_assert(FutureCost<MSTFutureCost>);

#endif
