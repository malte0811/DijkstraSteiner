#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <cstdint>
#include <limits>
#include <bitset>
#include <queue>
#include <array>
#include <cmath>

using Coord = std::uint32_t;
std::size_t constexpr num_dimensions = 3;
using Cost = Coord;
using Point = std::array<Coord, num_dimensions>;

using TerminalIndex = std::uint8_t;
TerminalIndex constexpr max_num_terminals = 20;
using TerminalSubset = std::bitset<max_num_terminals>;

using VertexIndex = std::uint16_t;
static_assert([]() constexpr -> std::size_t {
        auto const as_size_t = static_cast<std::size_t>(max_num_terminals);
        std::size_t result = 1;
        for (std::size_t i = 0; i < num_dimensions; ++i) {
            result *= as_size_t;
        }
        return result;
    }() < std::numeric_limits<VertexIndex>::max());

template<class T>
using MinHeap = std::priority_queue<T, std::vector<T>, std::greater<T>>;
//
// TODO remove once I have a working libstdc++ for C++20
// TODO do we even have one at the institute?
// copied from cppreference
template <class From, class To>
concept convertible_to =
  std::is_convertible_v<From, To> &&
  requires(std::add_rvalue_reference_t<From> (&f)()) {
    static_cast<To>(f());
  };


#endif
