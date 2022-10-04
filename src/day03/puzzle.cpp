#include "day03/puzzle.hpp"

#include <algorithm>
#include <charconv>
#include <deque>
#include <istream>
#include <numeric>
#include <optional>
#include <stdexcept>
#include <string>

#include "iosupport/iosupport.hpp"

namespace advent::day03 {

namespace {

int count01(int count, char ch) {
    if (ch == '0') {
        return count - 1;
    } else if (ch == '1') {
        return count + 1;
    }

    throw std::runtime_error("Invalid input character.");
}

int accumPositive(int accu, int value) { return 2 * accu + int(value > 0); }
int accumNegative(int accu, int value) { return 2 * accu + int(value < 0); }

} // namespace

void puzzleA(std::istream &input, std::ostream &output) {
    std::vector<int> counters;
    iosupport::loadFileInLines(input, [&counters](std::string_view line) {
        counters.resize(line.size());
        std::transform(counters.begin(), counters.end(), line.begin(),
                       counters.begin(), &count01);
    });

    const int gamma =
        std::accumulate(counters.begin(), counters.end(), 0, &accumPositive);
    const int epsilon =
        std::accumulate(counters.begin(), counters.end(), 0, &accumNegative);
    output << gamma * epsilon;
}

void puzzleB(std::istream &input, std::ostream &output) {}

} // namespace advent::day03
