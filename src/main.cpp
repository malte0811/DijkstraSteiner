#include <iostream>
#include "DijkstraSteiner.h"
#include "future_costs/NullFutureCost.h"
#include "future_costs/BBFutureCost.h"
#include <fstream>

int main(int argc, char** argv) {
    if (argc != 2) {
        return 1;
    }
    std::ifstream in(argv[1]);
    auto const optional_grid = HananGrid::read_from_stream(in);
    in.close();
    if (not optional_grid.has_value()) {
        return 1;
    }
    DijkstraSteiner<BBFutureCost> alg(optional_grid.value());
    auto const cost = alg.get_optimum_cost();
    std::cout << cost << '\n';
}
