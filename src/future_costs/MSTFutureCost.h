#ifndef MST_FUTURE_COST
#define MST_FUTURE_COST

#include "FutureCost.h"
#include <limits>
#include <unordered_map>

class MSTFutureCost {
public:
    explicit MSTFutureCost(HananGrid const& grid);

    Cost operator()(Label const& label) const;

    Cost compute_tree_cost(TerminalSubset const& not_contained_vertices) const;

private:
    using SingleVertexDistances = HananGrid::SingleVertexDistances;

    Cost get_tree_cost(TerminalSubset const& not_contained_vertices) const;

    HananGrid const& _grid;
    std::array<SingleVertexDistances, max_num_terminals> _costs{};
    // Index is the bitset in the corresponding label
    std::unordered_map<TerminalSubset, Cost> mutable _known_tree_costs;
    VertexIndex mutable _vertex_for_cached_distances = std::numeric_limits<VertexIndex>::max();
    SingleVertexDistances mutable _cached_distances{};
};

#endif
