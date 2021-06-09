#include <iostream>
#include "DijkstraSteiner.h"
#include "future_costs/NullFutureCost.h"
#include "future_costs/BBFutureCost.h"
#include <fstream>

int main(int argc, char** argv) {
    if (argc != 2) {
        return 1;
    }
    //TODO make safe!
    std::ifstream in(argv[1]);
    std::size_t num_terminals;
    in >> num_terminals;
    //TODO check num terminals
    assert(in);
    std::vector<Point> terminals;
    for (std::size_t index = 0; index < num_terminals; ++index) {
        Point new_point;
        for (std::size_t dim = 0; dim < num_dimensions; ++dim) {
            in >> new_point.at(dim);
            assert(in);
        }
        terminals.push_back(new_point);
    }
    HananGrid grid(terminals);;
    DijkstraSteiner<BBFutureCost> alg(grid);
    auto const cost = alg.get_optimum_cost();
    std::cout << cost << '\n';
}
