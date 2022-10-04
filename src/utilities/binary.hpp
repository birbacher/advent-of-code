#ifndef INCLUDED_ADVENT_UTILITIES_BINARY_HPP
#define INCLUDED_ADVENT_UTILITIES_BINARY_HPP

#include <string_view>

namespace utilities::binary {

int safeDouble(int v);

int accumBinary(int accu, char ch);

int readBinary(std::string_view sv);

} // namespace utilities::binary

#endif
