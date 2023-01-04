#include "adventofcode.hpp"

#include "iosupport/iosupport.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cctype>
#include <charconv>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <istream>
#include <iterator>
#include <numeric>
#include <optional>
#include <ostream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

namespace advent::common {

namespace {

using Z = std::int64_t;

Z readDigit(char c) {
    switch (c) {
    case '=':
        return -2;
    case '-':
        return -1;
    case '0':
        return 0;
    case '1':
        return 1;
    case '2':
        return 2;
    }
    throw std::runtime_error("Unknown character for SNAFU readDigit");
}

Z readSnafu(std::string_view sv) {
    return std::accumulate(sv.begin(), sv.end(), Z(0),
                           [](Z sofar, char currentDigit) {
                               return sofar * 5 + readDigit(currentDigit);
                           });
}

char snafuDigit(Z n) {
    switch (n) {
    case 3:
    case -2:
        return '=';
    case 4:
    case -1:
        return '-';
    case 0:
        return '0';
    case 1:
        return '1';
    case 2:
        return '2';
    }
    throw std::runtime_error("Unsupported value for snafuDigit");
}

std::string writeSnafu(Z n) {
    std::string tmp;
    Z rem = n;
    for (; rem > 0;) {
        Z m = (rem + 2) % 5 - 2;
        tmp += snafuDigit(m);
        rem -= m;
        rem /= 5;
    }
    std::reverse(tmp.begin(), tmp.end());
    return tmp;
}

} // namespace

template <> void puzzleA<2022, 25>(std::istream &input, std::ostream &output) {
    output << writeSnafu(
                  std::accumulate(std::istream_iterator<std::string>(input),
                                  std::istream_iterator<std::string>(), Z(0),
                                  [](Z sofar, std::string_view sv) {
                                      return sofar + readSnafu(sv);
                                  }))
           << '\n';
}

template <> void puzzleB<2022, 25>(std::istream &input, std::ostream &output) {}

} // namespace advent::common
