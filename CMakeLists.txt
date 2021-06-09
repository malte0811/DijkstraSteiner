cmake_minimum_required(VERSION 3.13)
project(DijkstraSteiner)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_COMPILER clang++-12)
add_definitions(-Wall -Wextra -pedantic -Werror)

add_executable(DijkstraSteiner
    src/main.cpp
    src/HananGrid.h src/HananGrid.cpp
    src/DijkstraSteiner.h
    src/LabelMap.h
    src/TypeDefs.h
    src/future_costs/NullFutureCost.h
    src/future_costs/BBFutureCost.h
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
