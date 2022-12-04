#include "adventofcode.hpp"

#include "iosupport/iosupport.hpp"

#include <algorithm>
#include <numeric>
#include <iterator>
#include <ostream>
#include <istream>
#include <stdexcept>
#include <string>

namespace advent::common {

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

int prioOfTriple(
    std::string_view rucksack1,
    std::string_view rucksack2,
    std::string_view rucksack3)
{
    std::array<int, orderedByPrio.size()> occurrences{};
    for (char c : rucksack1) {
        occurrences[getPriority(c) - 1] = 1;
    }
    for (char c : rucksack2) {
        int& occ = occurrences[getPriority(c) - 1];
        if (occ == 1) {
            occ = 2;
        }
    }
    for (char c : rucksack3) {
        int p = getPriority(c);
        if (occurrences[p - 1] == 2) {
            return p;
        }
    }
    throw std::runtime_error("No triples found");
}

struct Group {
    std::string rucksacks[3];
};

std::istream& operator>>(std::istream& stream, Group& g) {
    std::copy_n(std::istream_iterator<std::string>(stream), std::size(g.rucksacks), g.rucksacks);
    return stream;
}

}

template <>
void puzzleA<2022,3>(std::istream& input, std::ostream& output)
{
    int sum = 0;
    iosupport::loadFileInLines(input, [&](std::string_view sv) {
        auto sizeLhs = sv.size() / 2;
        sum += prioOfDuplicate({sv.data(), sizeLhs}, {sv.data() + sizeLhs, sv.size() - sizeLhs});
    });
    output << sum << '\n';
}

template <>
void puzzleB<2022,3>(std::istream& input, std::ostream& output)
{
    int sum = 0;
    std::for_each(std::istream_iterator<Group>(input), std::istream_iterator<Group>(), [&](Group const& g) {
        sum += prioOfTriple(g.rucksacks[0], g.rucksacks[1], g.rucksacks[2]);
    });
    output << sum << '\n';
}

}
