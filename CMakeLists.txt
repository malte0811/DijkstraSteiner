cmake_minimum_required(VERSION 3.13)
project(DijkstraSteiner)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_COMPILER clang++-12)
add_definitions(-Wall -Wextra -pedantic -Werror)

add_executable(DijkstraSteiner
    src/main.cpp
    src/HananGrid.h src/HananGrid.cpp
    src/DijkstraSteiner.h
        src/TypeDefs.h
    src/GridPoint.h
    src/future_costs/NullFutureCost.h
    src/future_costs/BBFutureCost.h src/future_costs/BBFutureCost.cpp
    src/future_costs/MSTFutureCost.h src/future_costs/MSTFutureCost.cpp
    src/SubsetIndexer.h
        src/PrimSteinerHeuristic.cpp src/PrimSteinerHeuristic.h)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
