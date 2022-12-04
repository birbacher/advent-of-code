#ifndef INCLUDED_ADVENT_ADVENTOFCODE_HPP
#define INCLUDED_ADVENT_ADVENTOFCODE_HPP

#include <iosfwd>

namespace advent::common {

template <int year, int day>
void puzzleA(std::istream& input, std::ostream& output);

template <int year, int day>
void puzzleB(std::istream& input, std::ostream& output);

}

#endif
