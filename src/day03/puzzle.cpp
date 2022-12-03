#include "day03/puzzle.hpp"

#include "iosupport/iosupport.hpp"

#include <algorithm>
#include <numeric>
#include <iterator>
#include <ostream>
#include <istream>
#include <stdexcept>
#include <string>

namespace advent::day03 {

namespace {

constexpr std::string_view orderedByPrio
    = "abcdefghijklmnopqrstuvwxyz"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

int getPriority(char c) {

    auto pos = std::find(orderedByPrio.begin(), orderedByPrio.end(), c);
    if (pos == orderedByPrio.end()) {
        using namespace std::literals;
        throw std::domain_error("Given character is not valid for 'getPriority': '"s + c + "'");
    }
    return 1 + std::distance(orderedByPrio.begin(), pos);
}

int prioOfDuplicate(std::string_view compartment1, std::string_view compartment2)
{
    std::array<bool, orderedByPrio.size()> occurrences{};
    for (char c : compartment1) {
        occurrences[getPriority(c) - 1] = true;
    }
    for (char c : compartment2) {
        int p = getPriority(c);
        if (occurrences[p - 1]) {
            return p;
        }
    }
    throw std::runtime_error("No duplicates found");
}

}

void puzzleA(std::istream& input, std::ostream& output)
{
    int sum = 0;
    iosupport::loadFileInLines(input, [&](std::string_view sv) {
        auto sizeLhs = sv.size() / 2;
        sum += prioOfDuplicate({sv.data(), sizeLhs}, {sv.data() + sizeLhs, sv.size() - sizeLhs});
    });
    output << sum << '\n';
}

void puzzleB(std::istream& input, std::ostream& output)
{
}

}
