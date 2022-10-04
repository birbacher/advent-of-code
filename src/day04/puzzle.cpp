#include "day04/puzzle.hpp"

#include <algorithm>
#include <charconv>
#include <deque>
#include <istream>
#include <limits>
#include <numeric>
#include <optional>
#include <stdexcept>
#include <string>
#include <list>

#include "iosupport/iosupport.hpp"
#include "utilities/binary.hpp"

namespace advent::day04 {

namespace {

std::vector<int> readLineAsCSV(std::istream &input)
{
    std::vector<int> numbers;
    for (;;) {
        int n;
        input >> n;
        if (!input) {
            throw std::runtime_error("failed to read number");
        }
        if (input.peek() != ',') {
            break;
        }
        input.get();
    }
    return numbers;
}

std::vector<int> readField5x5(std::istream &input) {
    std::vector<int> field(25);
    for (int &n : field) {
        input >> n;
    }
    return field;
}

} // namespace

void puzzleA(std::istream &input, std::ostream &output) {
    const auto numbers = readLineAsCSV(input);

    std::list<std::vector<int>> fields;
    for (;;) {
        auto f = readField5x5(input);
        if (!input) {
            break;
        }
        fields.push_back(std::move(f));
    }

    output << fields.size() << '\n';
}

void puzzleB(std::istream &input, std::ostream &output) {
}

} // namespace advent::day04
