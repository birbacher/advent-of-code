#include "day03/puzzle.hpp"

#include <algorithm>
#include <charconv>
#include <deque>
#include <istream>
#include <limits>
#include <numeric>
#include <optional>
#include <stdexcept>
#include <string>

#include "iosupport/iosupport.hpp"
#include "utilities/binary.hpp"

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

int accumPositive(int accu, int value) {
    return utilities::binary::safeDouble(accu) + int(value > 0);
}
int accumNegative(int accu, int value) {
    return utilities::binary::safeDouble(accu) + int(value < 0);
}

struct part {
    int i;
    part(int i) : i(i) {}
    bool operator()(std::string_view sv) const { return sv.at(i) == '0'; }
};

template <typename ITER, typename CMP>
ITER findEntry(ITER first, ITER last, CMP cmp) {
    int index = 0;
    for (;; ++index) {
        if (first == last) {
            throw std::runtime_error("exhausted search space");
        }
        if (first + 1 == last) {
            return first;
        }
        auto pos = std::partition_point(first, last, part(index));
        const int count0 = pos - first;
        const int count1 = last - pos;
        (cmp(count0, count1) ? last : first) = pos;
    }
}

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

void puzzleB(std::istream &input, std::ostream &output) {
    auto lines = iosupport::loadFileInLines(input);
    std::sort(lines.begin(), lines.end());

    const auto oxygenRatingPos =
        findEntry(lines.begin(), lines.end(), std::greater<>{});
    const auto co2ScrubberPos =
        findEntry(lines.begin(), lines.end(), std::less_equal<>{});

    output << utilities::binary::readBinary(*oxygenRatingPos) * utilities::binary::readBinary(*co2ScrubberPos)
           << '\n';
}

} // namespace advent::day03
